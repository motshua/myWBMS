/* Local files */
#include "bms.h"
#include "dashboard.h"
#include "html.h"

/* External libraries */
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <credentials.h>

/* Local functions */
void send_msg_to(int bmm);
void save_bmm_data();
char* get_mac_addr_str(uint8_t * mac);

uint8_t bmu_address[] = {0xf0, 0x08, 0xd1, 0xc8, 0x76, 0xf8};
uint8_t bmm_0_address[] = {0x7c, 0x9e, 0xbd, 0xe3, 0x9b, 0xc4};
String bmm_0_address_str = "7c:9e:bd:e3:9b:c4";
uint8_t bmm_1_address[] = {0xf0, 0x08, 0xd1, 0xc7, 0xa8, 0xb4};
String bmm_1_address_str = "f0:08:d1:c7:a8:b4";
uint8_t bmm_2_address[] = {0x7c, 0x9e, 0xbd, 0xe3, 0x0f, 0x94};
String bmm_2_address_str = "7c:9e:bd:e3:0f:94";

/* Message counter */
long msg_count = 0;

/* Interval cycle between the data request of one module to the other */
const unsigned int r_i_b_m = 30; // request interval between modules
const unsigned int bmm_start_interval[] = {r_i_b_m * 0, r_i_b_m * 1, r_i_b_m * 2};
const unsigned int bmm_end_interval[]   = {r_i_b_m * 1, r_i_b_m * 2, r_i_b_m * 3};

String this_second = now_str();
bool this_second_is_updated = 0;

typedef struct struct_bmu {
    float max_v; // to turn on balancing circuits
    float min_v; // if below min_v, enter eco mode (dont turn on wifi and check v every 10s or more)
    float max_b_t; // max balancing temp
    bool chrg_state;
    float lowest_v;
    long sync;
    long msg_count;
} struct_bmu;

typedef struct struct_bmm {
    int id;
    float c1_v;
    float c1_t;
    int c1_b_p;
    float c1_b_t;
    float c2_v;
    float c2_t;
    int c2_b_p;
    float c2_b_t;
    float c3_v;
    float c3_t;
    int c3_b_p;
    float c3_b_t;
    String err;
    int time_awake;
    int time_waiting;
    long msg_count;
} struct_bmm;

struct_bmu bmu_out;
struct_bmm bmm_in;

/* Callback when data is sent */
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("Last Packet Send Status: ");
  //Serial.print(status == ESP_NOW_SEND_SUCCESS ? "Delivery success, millis: " : "Delivery fail, millis: ");
  //Serial.println(millis() % req_bmm_interval);
  if (status == 0){
    //success = "Delivery success";
  }
  else{
    //success = "Delivery fail";
  }
}

/* Callback when data is received */
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&bmm_in, incomingData, sizeof(bmm_in));
  //Serial.printf("Received %i bytes from ", len);
  String mac_str = get_mac_addr_str((uint8_t *)mac);
  if (mac_str == bmm_0_address_str) {
    new_msg_bmm_0 = 1;
    //Serial.printf("BMM_0 (millis: %i)\n", millis() % req_bmm_interval);
  } else if (mac_str == bmm_1_address_str) {
    new_msg_bmm_1 = 1;
    //Serial.printf("BMM_1 (millis: %i)\n", millis() % req_bmm_interval);
  } else if (mac_str == bmm_2_address_str) {
    new_msg_bmm_2 = 1;
    //Serial.printf("BMM_2 (millis: %i)\n", millis() % req_bmm_interval);
  }
}

char mac_addr_str[18];
char* get_mac_addr_str(uint8_t * mac) {
  snprintf( mac_addr_str,
            sizeof(mac_addr_str),
            "%02x:%02x:%02x:%02x:%02x:%02x",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
          );
  return mac_addr_str;
}

/* Prepare BMU message to be send to the modules */
void prepare_bmu_msg() {

  if ( (millis() % req_bmm_interval > req_bmm_interval - 100) && (bmu_msg_ready == 0) ) {
    
    //Serial.println("");
    //Serial.printf("Preparing BMU msg (millis: %i)...\n", millis() % req_bmm_interval);
    bmu_out.max_v = read_file(SPIFFS, "/max_cell_voltage.txt").toFloat();
    bmu_out.min_v = read_file(SPIFFS, "/min_cell_voltage.txt").toFloat();
    bmu_out.max_b_t = read_file(SPIFFS, "/max_cell_bal_temp.txt").toFloat();
    bmu_out.chrg_state = charging_current >= 0.100 ? 1 : 0;
    bmu_out.lowest_v = lowest_cell_voltage;
    bmu_out.msg_count = msg_count;
    //Serial.printf("BMU msg ready (millis: %i)...\n", millis() % req_bmm_interval);
    bmu_msg_ready = 1;
  }
}

/* Send request message to the modules */
void send_msg_to_bmms() {
  
  if ( millis() % req_bmm_interval >= bmm_start_interval[0] && millis() % req_bmm_interval < bmm_end_interval[0] && requested[0] == 0 ) {
    send_msg_to(bmm_0);
  }
  if ( (millis() % req_bmm_interval >= bmm_start_interval[1] || data_ready[bmm_0] == 1) && millis() % req_bmm_interval < bmm_end_interval[1] && requested[1] == 0 && requested[0] == 1 ) {
    send_msg_to(bmm_1);
  }
  if ( (millis() % req_bmm_interval >= bmm_start_interval[2] || data_ready[bmm_1] == 1) && millis() % req_bmm_interval < bmm_end_interval[2] && requested[2] == 0 && requested[1] == 1 ) {
    send_msg_to(bmm_2);
  }
}

void send_msg_to(int bmm) {

  uint8_t bmm_address[sizeof(bmm_0_address)];

  if (bmm == bmm_0) { memcpy(bmm_address, bmm_0_address, sizeof(bmm_0_address)); }
  if (bmm == bmm_1) { memcpy(bmm_address, bmm_1_address, sizeof(bmm_1_address)); }
  if (bmm == bmm_2) { memcpy(bmm_address, bmm_2_address, sizeof(bmm_2_address)); }

  //Serial.printf("Sending msg to BMM_%i (mac: %s) (millis: %i)...\n", bmm, get_mac_addr_str(bmm_address), millis() % req_bmm_interval);

  bmu_out.sync = millis() % 1000 - bmm_start_interval[bmm];
  
  esp_err_t result = esp_now_send(bmm_address, (uint8_t *) &bmu_out, sizeof(bmu_out));
  if (result == ESP_OK) {
    //Serial.printf("Sent with success. (millis: %i)\n", millis() % req_bmm_interval);
    requested[bmm] = 1;
  } else {
    //Serial.printf("Error sending the data. (millis: %i)\n", millis() % req_bmm_interval);
    delay(1);
  }
}

/* If any of the modules failed to send a message back, request again */
void request_again() {
  
  // if ( millis() % req_bmm_interval >= 100 ) {
    
  //   if ( (millis() % req_bmm_interval) % 100 >= bmm_start_interval[0] && (millis() % req_bmm_interval) % 100 < bmm_end_interval[0] && received_failed[0] == 1 ) {
  //     send_msg_to(bmm_0);
  //   }
  //   if ( (millis() % req_bmm_interval) % 100 >= bmm_start_interval[1] && (millis() % req_bmm_interval) % 100 < bmm_end_interval[1] && received_failed[1] == 1 ) {
  //     send_msg_to(bmm_1);
  //   }
  //   if ( (millis() % req_bmm_interval) % 100 >= bmm_start_interval[2] && (millis() % req_bmm_interval) % 100 < bmm_end_interval[2] && received_failed[2] == 1 ) {
  //     send_msg_to(bmm_2);
  //   }
  // }
}

void save_bmm_data() {

  if (!this_second_is_updated) {
    this_second = now_str();
    this_second_is_updated = 1;
  }

  if (new_msg_bmm_0 == 1) {

    //Serial.printf("Saving BMM_%i data (millis: %i)...\n", bmm_in.id, millis() % req_bmm_interval);

    json_bmm_0["error_code"] = bmm_in.err;
    json_bmm_0["cell_1_voltage"] = bmm_in.c1_v;
    json_bmm_0["cell_2_voltage"] = bmm_in.c2_v;
    json_bmm_0["cell_3_voltage"] = bmm_in.c3_v;
    json_bmm_0["cell_1_temp"] = bmm_in.c1_t;
    json_bmm_0["cell_2_temp"] = bmm_in.c2_t;
    json_bmm_0["cell_3_temp"] = bmm_in.c3_t;
    json_bmm_0["cell_1_bal_state"] = (bmm_in.c1_b_p == 100 ? "ON" : "OFF");
    json_bmm_0["cell_2_bal_state"] = (bmm_in.c2_b_p == 100 ? "ON" : "OFF");
    json_bmm_0["cell_3_bal_state"] = (bmm_in.c3_b_p == 100 ? "ON" : "OFF");
    json_bmm_0["cell_1_bal_temp"] = bmm_in.c1_b_t;
    json_bmm_0["cell_2_bal_temp"] = bmm_in.c2_b_t;
    json_bmm_0["cell_3_bal_temp"] = bmm_in.c3_b_t;
    json_bmm_0["time_awake"] = bmm_in.time_awake;
    json_bmm_0["time_waiting"] = bmm_in.time_waiting;
    json_bmm_0["last_msg"] = this_second;
    json_bmm_0["mac_bmm"] = bmm_0_address_str;

    cell_voltage[0] = bmm_in.c1_v;
    cell_voltage[1] = bmm_in.c2_v;
    cell_voltage[2] = bmm_in.c3_v;
    cell_temp[0] = bmm_in.c1_t;
    cell_temp[1] = bmm_in.c2_t;
    cell_temp[2] = bmm_in.c3_t;

    //Serial.printf("BMM_%i data saved (millis: %i)...\n", bmm_in.id, millis() % req_bmm_interval);

    received_failed[bmm_0] = 0;
    data_ready[bmm_0] = 1;

    new_msg_bmm_0 = 0;
  } 
  
  else if (new_msg_bmm_1 == 1) {
    
    //Serial.printf("Saving BMM_%i data (millis: %i)...\n", bmm_in.id, millis() % req_bmm_interval);

    json_bmm_1["error_code"] = bmm_in.err;
    json_bmm_1["cell_1_voltage"] = bmm_in.c1_v;
    json_bmm_1["cell_2_voltage"] = bmm_in.c2_v;
    json_bmm_1["cell_3_voltage"] = bmm_in.c3_v;
    json_bmm_1["cell_1_temp"] = bmm_in.c1_t;
    json_bmm_1["cell_2_temp"] = bmm_in.c2_t;
    json_bmm_1["cell_3_temp"] = bmm_in.c3_t;
    json_bmm_1["cell_1_bal_state"] = (bmm_in.c1_b_p == 100 ? "ON" : "OFF");
    json_bmm_1["cell_2_bal_state"] = (bmm_in.c2_b_p == 100 ? "ON" : "OFF");
    json_bmm_1["cell_3_bal_state"] = (bmm_in.c3_b_p == 100 ? "ON" : "OFF");
    json_bmm_1["cell_1_bal_temp"] = bmm_in.c1_b_t;
    json_bmm_1["cell_2_bal_temp"] = bmm_in.c2_b_t;
    json_bmm_1["cell_3_bal_temp"] = bmm_in.c3_b_t;
    json_bmm_1["time_awake"] = bmm_in.time_awake;
    json_bmm_1["time_waiting"] = bmm_in.time_waiting;
    json_bmm_1["last_msg"] = this_second;
    json_bmm_1["mac_bmm"] = bmm_1_address_str;

    cell_voltage[3] = bmm_in.c1_v;
    cell_voltage[4] = bmm_in.c2_v;
    cell_voltage[5] = bmm_in.c3_v;
    cell_temp[3] = bmm_in.c1_t;
    cell_temp[4] = bmm_in.c2_t;
    cell_temp[5] = bmm_in.c3_t;

    //Serial.printf("BMM_%i data saved (millis: %i)...\n", bmm_in.id, millis() % req_bmm_interval);

    received_failed[bmm_1] = 0;
    data_ready[bmm_1] = 1;

    new_msg_bmm_1 = 0;
  }

  else if (new_msg_bmm_2 == 1) {
    
    //Serial.printf("Saving BMM_%i data (millis: %i)...\n", bmm_in.id, millis() % req_bmm_interval);

    json_bmm_2["error_code"] = bmm_in.err;
    json_bmm_2["cell_1_voltage"] = bmm_in.c1_v;
    json_bmm_2["cell_2_voltage"] = bmm_in.c2_v;
    json_bmm_2["cell_3_voltage"] = bmm_in.c3_v;
    json_bmm_2["cell_1_temp"] = bmm_in.c1_t;
    json_bmm_2["cell_2_temp"] = bmm_in.c2_t;
    json_bmm_2["cell_3_temp"] = bmm_in.c3_t;
    json_bmm_2["cell_1_bal_state"] = (bmm_in.c1_b_p == 100 ? "ON" : "OFF");
    json_bmm_2["cell_2_bal_state"] = (bmm_in.c2_b_p == 100 ? "ON" : "OFF");
    json_bmm_2["cell_3_bal_state"] = (bmm_in.c3_b_p == 100 ? "ON" : "OFF");
    json_bmm_2["cell_1_bal_temp"] = bmm_in.c1_b_t;
    json_bmm_2["cell_2_bal_temp"] = bmm_in.c2_b_t;
    json_bmm_2["cell_3_bal_temp"] = bmm_in.c3_b_t;
    json_bmm_2["time_awake"] = bmm_in.time_awake;
    json_bmm_2["time_waiting"] = bmm_in.time_waiting;
    json_bmm_2["last_msg"] = this_second;
    json_bmm_2["mac_bmm"] = bmm_2_address_str;

    cell_voltage[6] = bmm_in.c1_v;
    cell_voltage[7] = bmm_in.c2_v;
    cell_voltage[8] = bmm_in.c3_v;
    cell_temp[6] = bmm_in.c1_t;
    cell_temp[7] = bmm_in.c2_t;
    cell_temp[8] = bmm_in.c3_t;

    //Serial.printf("BMM_%i data saved (millis: %i)...\n", bmm_in.id, millis() % req_bmm_interval);

    received_failed[bmm_2] = 0;
    data_ready[bmm_2] = 1;

    new_msg_bmm_2 = 0;
  }

}

void update_bmm_data_on_dashboard() {

  if (millis() % req_bmm_interval >= 100) {
    if (data_ready[bmm_0] == 1) {
      //Serial.printf("Updating BMM_0 data on dashboard (millis: %i)...\n", millis() % req_bmm_interval);
      String jsonString = JSON.stringify(json_bmm_0);
      events.send(jsonString.c_str(), "new_bmm_0", millis());
      //Serial.printf("Dashboard updated (millis: %i)...\n", millis() % req_bmm_interval);
      data_ready[bmm_0] = 0;
    }
    if (data_ready[bmm_1] == 1) {
      //Serial.printf("Updating BMM_1 data on dashboard (millis: %i)...\n", millis() % req_bmm_interval);
      String jsonString = JSON.stringify(json_bmm_1);
      events.send(jsonString.c_str(), "new_bmm_1", millis());
      //Serial.printf("Dashboard updated (millis: %i)...\n", millis() % req_bmm_interval);
      data_ready[bmm_1] = 0;
    }
    if (data_ready[bmm_2] == 1) {
      //Serial.printf("Updating BMM_2 data on dashboard (millis: %i)...\n", millis() % req_bmm_interval);
      String jsonString = JSON.stringify(json_bmm_2);
      events.send(jsonString.c_str(), "new_bmm_2", millis());
      //Serial.printf("Dashboard updated (millis: %i)...\n", millis() % req_bmm_interval);
      data_ready[bmm_2] = 0;
    }
  }
}

void update_bmu_data_on_dashboard() {

  if (data_ready[bmu]) {
    
    //events.send("ping",NULL,millis());

    //Serial.printf("Updating BMU data on dashboard (millis: %i)...\n", millis() % req_bmm_interval);

    json_bmu["pack_voltage"] = bmm_pack_voltage;
    json_bmu["pack_voltage_BMU"] = bmu_pack_voltage;
    json_bmu["charging_voltage"] = charging_voltage;
    json_bmu["avg_cell_voltage"] = bmm_pack_voltage/num_cells_pack;

    json_bmu["pack_state_of_charge"] = state_of_charge_mah;
    json_bmu["pack_state_of_charge_perc"] = state_of_charge_perc;
    json_bmu["pack_charge_capacity"] = charge_capacity_mah;
    json_bmu["pack_state_of_energy"] = state_of_energy_mwh;
    json_bmu["pack_state_of_energy_perc"] = state_of_energy_perc;
    json_bmu["pack_energy_capacity"] = energy_capacity_mwh;

    // Serial.printf("\n\nstate_of_charge_mah: %.2f\n", state_of_charge_mah);
    // Serial.printf("state_of_charge_perc: %.2f\n", state_of_charge_perc);
    // Serial.printf("charge_capacity_mah: %.2f\n", charge_capacity_mah);
    // Serial.printf("state_of_energy_mwh: %.2f\n", state_of_energy_mwh);
    // Serial.printf("state_of_energy_perc: %.2f\n", state_of_energy_perc);
    // Serial.printf("energy_capacity_mwh: %.2f\n", energy_capacity_mwh);

    json_bmu["load_current"] = load_current;
    json_bmu["charging_current"] = charging_current;
    json_bmu["min_cell_voltage"] = read_file(SPIFFS, "/min_cell_voltage.txt").toFloat();
    json_bmu["min_cell_voltage_release"] = read_file(SPIFFS, "/min_cell_voltage_release.txt").toFloat();
    json_bmu["max_cell_voltage"] = read_file(SPIFFS, "/max_cell_voltage.txt").toFloat();
    json_bmu["max_cell_voltage_release"] = read_file(SPIFFS, "/max_cell_voltage_release.txt").toFloat();
    json_bmu["max_load_current"] = read_file(SPIFFS, "/max_load_current.txt").toFloat();
    json_bmu["max_charging_current"] = read_file(SPIFFS, "/max_charging_current.txt").toFloat();
    json_bmu["min_cell_temp"] = read_file(SPIFFS, "/min_cell_temp.txt").toFloat();
    json_bmu["max_cell_temp"] = read_file(SPIFFS, "/max_cell_temp.txt").toFloat();
    json_bmu["max_cell_bal_temp"] = read_file(SPIFFS, "/max_cell_bal_temp.txt").toFloat();
    json_bmu["series_cell_num"] = num_cells_pack;
    json_bmu["lowest_cell_voltage"] = lowest_cell_voltage;
    json_bmu["lowest_cell_voltage_index"] = lowest_cell_voltage_index;
    json_bmu["highest_cell_voltage"] = highest_cell_voltage;
    json_bmu["highest_cell_voltage_index"] = highest_cell_voltage_index;
    json_bmu["charging_switch_state"] = (charging_switch_state == 1 ? "ON" : "OFF");
    json_bmu["discharging_switch_state"] = (load_switch_state == 1 ? "ON" : "OFF");
    json_bmu["error_code_bmu"] = "NULL";
    json_bmu["mac_bmu"] = get_mac_addr_str(bmu_address);
    json_bmu["eject_sd_card"] = eject_sd_card == 1 ? "yes" : "no";
    json_bmu["last_msg_bmu"] = now_str();
    //json_bmu["msg_id"] = msg_id;

    String jsonString = JSON.stringify(json_bmu);

    events.send(jsonString.c_str(), "new_bmu", millis());

    json_lang["lang"] = read_file(SPIFFS, "/lang.txt");
    jsonString = JSON.stringify(json_lang);
    events.send(jsonString.c_str(), "lang", millis());

    //Serial.printf("Dashboard updated (millis: %i)...\n", millis() % req_bmm_interval);

    data_ready[bmu] = 0;
  }
}

void reset_flags() {

  if ( (millis() % req_bmm_interval >= req_bmm_interval - 110) && (millis() % req_bmm_interval < req_bmm_interval - 100) ) { //&& requested[0] && requested[1] && requested[2] ) {
    requested[bmm_0] = 0;
    requested[bmm_1] = 0;
    requested[bmm_2] = 0;
    received_failed[bmm_0] = 1;
    received_failed[bmm_1] = 1;
    received_failed[bmm_2] = 1;
    data_ready[bmm_0] = 0;
    data_ready[bmm_1] = 0;
    data_ready[bmm_2] = 0;
    data_ready[bmu] = 0;
    bmu_msg_ready = 0;
    bmu_data_computed = 0;
    this_second_is_updated = 0;
    msg_count++;
  }
}
