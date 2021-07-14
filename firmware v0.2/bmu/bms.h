/* External libraries */
#include <NTPClient.h> // to get time from NTP Server
#include <WiFiUdp.h>
#include "SPI.h" // for ADC128S102
#include <Adafruit_NeoPixel.h> // for WS2812
#include <Wire.h> // I2C
#include "RTClib.h" // for DS3231
#include "SD.h" // for SD card
#include "FS.h"
#include <SPI.h>

/* ESP32 Pins */
#define load_mosfet_pin 16
#define charging_mosfet_pin 17
#define sd_cs_pin 15
#define adc_cs_pin 5
#define ldo_5v_en_pin 33

/* ADC Pins */
#define adc_charging_current_pin 3
#define adc_charging_voltage_pin 4
#define adc_load_current_pin 5
#define adc_pack_voltage_pin 6

/* Local functions */
void calibrate_current_sensors();
void get_readings();
float get_bmu_pack_voltage();
float get_charging_voltage();
float get_bmm_pack_voltage();
float get_load_current_without_calibration();
float get_load_current();
float get_charging_current_without_calibration();
float get_charging_current();
void getTimeStamp();
void log_to_sd_card();
String now_str();
void compute_bmu_data();
void get_highest_lowest_cell_voltage();
void get_highest_lowest_cell_temp();
void get_charging_switch_state();
void set_charging_switch_state();
void get_load_switch_state();
void set_load_switch_state();
void compute_soc_soe(float current_in, float current_out, float voltage, float integration_interval);
void write_file(fs::FS &fs, const char * path, const char * message);
void append_file(fs::FS &fs, const char * path, const char * message);
int read_adc(int channel);
void get_adc();
float get_system_avg_current_consumption();
void system_led();

/* Flags for esp-now */
bool requested[] = {0, 0, 0};
bool received_failed[] = {1, 1, 1};
bool bmu_msg_ready = 0;
bool bmu_data_computed = 0;
bool new_msg_bmm_0 = 0;
bool new_msg_bmm_1 = 0;
bool new_msg_bmm_2 = 0;
/* Interval cycle at which data is requested from all BMMs */
const unsigned int req_bmm_interval = 1000;

/* Module index */
uint8_t bmm_0 = 0;
uint8_t bmm_1 = 1;
uint8_t bmm_2 = 2;
uint8_t bmu = 3;

/* Flags for data state */
bool data_ready[] = {0, 0, 0, 0}; // bmm_0, bmm_1, bmm_2 and bmu

/* Variables to save date and time */
String formatted_date;
int date_day;
int date_month;
int date_year;
int time_second;
int time_minute;
int time_hour;
DateTime now; 

/* BMS variables */
const int num_modules = 3;
const int num_cells_per_module = 3;
const int num_cells_pack = num_modules*num_cells_per_module;
float system_avg_current_consumption = 0;
float bmu_pack_voltage = 0.00;
float charging_voltage = 0.00;
float bmm_pack_voltage = 0.00;
const int current_h_size = 4;
float load_current = 0.00;
float load_current_h[current_h_size];
float charging_current = 0.00;
float charging_current_h[current_h_size];
float load_current_offset = 0.00;
float charging_current_offset = 0.00;
float cell_voltage[num_cells_pack];
float highest_cell_voltage = 0.00;
int highest_cell_voltage_index = 0;
float lowest_cell_voltage = 0.00;
int lowest_cell_voltage_index = 0;
float cell_temp[num_cells_pack];
float cell_bal_temp[num_cells_pack];
float highest_cell_temp = 0.00;
float lowest_cell_temp = 0.00;
bool load_switch_state = 0;
bool charging_switch_state = 0;
float min_cell_voltage = 0.00;
float min_cell_voltage_release = 0.00;
float max_cell_voltage = 0.00;
float max_cell_voltage_release = 0.00;
float max_load_current = 0.00;
float max_charging_current = 0.00;
float min_cell_temp = 0.00;
float max_cell_temp = 0.00;
float max_cell_bal_temp = 0.00;
float charge_capacity_mah = 1000.00;
float state_of_charge_mah = 0.00;
float state_of_charge_perc = 0.00;
float energy_capacity_mwh = 1000.00;
float state_of_energy_mwh = 0.00;
float state_of_energy_perc = 0.00;
bool sd_card_initialized = 1;
bool eject_sd_card = 0;
int sd_log_counter = 0;
bool battery_reached_bottom = 0;
bool sys_led_flag = 0;

/* Define NTP Client to get time */
WiFiUDP ntpUDP;
NTPClient time_client(ntpUDP);

/* for DS3231 */
RTC_DS3231 rtc;
char days_of_the_week[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

/* for WS2812 */
Adafruit_NeoPixel pixels(1, 14, NEO_GRB + NEO_KHZ800);

void compute_bmu_data() {
  
  if (requested[bmm_0] && requested[bmm_1] && requested[bmm_2] && data_ready[bmu] == 0 && bmu_data_computed == 0 && millis() % req_bmm_interval >= 200) {
    
    //Serial.printf("Computing BMU data (millis: %i)...\n", millis() % req_bmm_interval);
    get_readings();
    get_highest_lowest_cell_voltage();
    get_highest_lowest_cell_temp();
    get_charging_switch_state();
    set_charging_switch_state();
    get_load_switch_state();
    set_load_switch_state();
    compute_soc_soe(charging_current, load_current, bmm_pack_voltage, 1000 /*interval [ms]*/);
    //log_to_sd_card();
    data_ready[bmu] = 1;
    bmu_data_computed = 1;
  }
}

void calibrate_current_sensors() {
  
  /* Setting the state of the mosfets LOW */
  digitalWrite(charging_mosfet_pin, LOW);
  digitalWrite(load_mosfet_pin, LOW);

  float load_current = get_load_current_without_calibration();
  float charging_current = get_charging_current_without_calibration();

  // Serial.print("Load current before calibration: ");
  // Serial.print(load_current, 2);
  // Serial.println(" A");
  // Serial.print("Charging current before calibration: ");
  // Serial.print(charging_current, 2);
  // Serial.println(" A");

  load_current_offset = 0 - load_current;
  charging_current_offset = 0 - charging_current;

  load_current = get_load_current();
  charging_current = get_charging_current();

  // Serial.print("Load current after calibration: ");
  // Serial.print(load_current, 2);
  // Serial.println(" A");
  // Serial.print("Charging current after calibration: ");
  // Serial.print(charging_current, 2);
  // Serial.println(" A");
}

void get_readings() {
  
  //Serial.printf("Getting readings (millis: %i)...\n", millis() % req_bmm_interval);
  get_bmu_pack_voltage();
  get_charging_voltage();
  get_bmm_pack_voltage();
  get_load_current();
  get_charging_current();

}

float get_bmu_pack_voltage() {
  
  //Serial.printf("Getting bmu_pack_voltage (millis: %i)...\n", millis() % req_bmm_interval);
  bmu_pack_voltage = read_adc(adc_pack_voltage_pin);
  bmu_pack_voltage = map(bmu_pack_voltage, 0, 4095, 0, 5000);
  bmu_pack_voltage = bmu_pack_voltage * 15; // voltage divider (1/15)
  bmu_pack_voltage = bmu_pack_voltage/1000;
  return bmu_pack_voltage;
}

float get_charging_voltage() {
  
  //Serial.printf("Getting charging_voltage (millis: %i)...\n", millis() % req_bmm_interval);
  charging_voltage = read_adc(adc_charging_voltage_pin);
  charging_voltage = map(charging_voltage, 0, 4095, 0, 5000);
  charging_voltage = charging_voltage * 15; // voltage divider (1/15)
  charging_voltage = charging_voltage/1000;
  return charging_voltage;
}

float get_bmm_pack_voltage() {
  
  //Serial.printf("Getting bmm_pack_voltage (millis: %i)...\n", millis() % req_bmm_interval);
  bmm_pack_voltage = 0;
  for (int i = 0; i < num_cells_pack; i++) {
    bmm_pack_voltage = bmm_pack_voltage + cell_voltage[i];
  }
  return bmm_pack_voltage;
}

float get_load_current_without_calibration() {
  
  load_current = 0;
  const int sample_size = 100;
  for(int i = 0; i < sample_size; i++)
  {
    load_current += read_adc(adc_load_current_pin);
    delayMicroseconds(100);
  }
  load_current = load_current / sample_size;
  load_current = map(load_current, 0, 4095, 0, 5000);
  load_current = load_current * 1.025; // voltage divider (100/102.5)
  load_current = load_current - 2500; // 0 A value is at VCC/2
  load_current = load_current / 185; // ACS712-5A: 185 mV/A
  return load_current;
}

float get_load_current() {
  
  //Serial.printf("Getting load_current (millis: %i)...\n", millis() % req_bmm_interval);
  load_current = 0;
  const int sample_size = 10;
  for(int i = 0; i < sample_size; i++) {
    load_current += read_adc(adc_load_current_pin);
    delayMicroseconds(100);
  }
  load_current = load_current / sample_size;
  load_current = map(load_current, 0, 4095, 0, 5000);
  load_current = load_current * 1.025; // voltage divider (100/102.5)
  load_current = load_current - 2500; // 0 A value is at VCC/2
  load_current = load_current / 185; // ACS712-5A: 185 mV/A
  load_current = load_current + load_current_offset; // offset
  //load_current = (load_current - 0.0) * 0.95;
  if (load_current*1000 < 50) {
    //Serial.printf("load_current = %.2f A < 50 mA\n", load_current);
    load_current = 0;
  }
  for(int i = 0; i < current_h_size; i++) {
    if (i+1 < current_h_size) {
      load_current_h[i+1] = load_current_h[i];
    }
  }
  load_current_h[0] = load_current;
  load_current = 0;
  for(int i = 0; i < current_h_size; i++) {
    load_current = load_current + load_current_h[i];
  }
  load_current = load_current/current_h_size;
  load_current_h[0] = load_current;
  return load_current;
}

float get_charging_current_without_calibration() {
  
  charging_current = 0;
  const int sample_size = 100;
  for(int i = 0; i < sample_size; i++) {
    charging_current += read_adc(adc_charging_current_pin);
    delayMicroseconds(100);
  }
  charging_current = charging_current / sample_size;
  charging_current = map(charging_current, 0, 4095, 0, 5000);
  charging_current = charging_current * 1.025; // voltage divider (100/102.5)
  charging_current = charging_current - 2500; // 0 A value is at VCC/2
  charging_current = charging_current / 185; // ACS712-5A: 185 mV/A
  return charging_current;
}

float get_charging_current() {
  
  //Serial.printf("Getting charging_current (millis: %i)...\n", millis() % req_bmm_interval);
  charging_current = 0;
  const int sample_size = 10;
  for(int i = 0; i < sample_size; i++) {
    charging_current += read_adc(adc_charging_current_pin);
    delayMicroseconds(100);
  }
  charging_current = charging_current / sample_size;
  charging_current = map(charging_current, 0, 4095, 0, 5000);
  charging_current = charging_current * 1.025; // voltage divider (100/102.5)
  charging_current = charging_current - 2500; // 0 A value is at VCC/2
  charging_current = charging_current / 185; // ACS712-5A: 185 mV/A
  charging_current = charging_current + charging_current_offset; // offset
  //charging_current = (charging_current - 0.0) * 0.95;
  if (charging_current*1000 < 50) {
    //Serial.printf("charging_current = %.2f A < 50 mA\n", charging_current);
    charging_current = 0;
  }
  for(int i = 0; i < current_h_size; i++) {
    if (i+1 < current_h_size) {
      charging_current_h[i+1] = charging_current_h[i];
    }
  }
  charging_current_h[0] = charging_current;
  charging_current = 0;
  for(int i = 0; i < current_h_size; i++) {
    charging_current = charging_current + charging_current_h[i];
  }
  charging_current = charging_current/current_h_size;
  charging_current_h[0] = charging_current;
  return charging_current;
}

void getTimeStamp() {
  
  //while(!time_client.update()) {
  //  time_client.forceUpdate();
  //}
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  //formattedDate = time_client.getFormattedDate();
  //Serial.println(formattedDate);

  // Extract date
  //int splitT = formattedDate.indexOf("T");
  //dayStamp = formattedDate.substring(0, splitT);
  //Serial.println(dayStamp);
  // Extract time
  //timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  //Serial.println(timeStamp);
}

String now_str() {
  
  char now_str[20];
  now = rtc.now();
  snprintf( now_str,
            sizeof(now_str),
            "%02d/%02d/%02d %02d:%02d:%02d",
            now.day(), now.month(), now.year(), now.hour(), now.minute(), now.second()
          );
  return now_str;
}

void get_highest_lowest_cell_voltage() {

  //Serial.printf("Getting highest_lowest_cell_voltage (millis: %i)...\n", millis() % req_bmm_interval);
  highest_cell_voltage = 0;
  for (int i = 0; i < num_cells_pack; i++) {
    if (cell_voltage[i] > highest_cell_voltage) {
      highest_cell_voltage = cell_voltage[i];
      highest_cell_voltage_index = i+1;
    }
  }
  lowest_cell_voltage = highest_cell_voltage;
  for (int i = 0; i < num_cells_pack; i++) {
    if (cell_voltage[i] < lowest_cell_voltage) {
      lowest_cell_voltage = cell_voltage[i];
      lowest_cell_voltage_index = i+1;
    }
  }
}

void get_highest_lowest_cell_temp() {

  //Serial.printf("Getting highest_lowest_cell_temp (millis: %i)...\n", millis() % req_bmm_interval);
  highest_cell_temp = 0;
  for (int i = 0; i < num_cells_pack; i++) {
    if (cell_temp[i] > highest_cell_temp)
      highest_cell_temp = cell_temp[i];
  }
  lowest_cell_temp = highest_cell_temp;
  for (int i = 0; i < num_cells_pack; i++) {
    if (cell_temp[i] < lowest_cell_temp)
      lowest_cell_temp = cell_temp[i];
  }
}

void get_charging_switch_state() {

  //Serial.printf("Getting charging_switch_state (millis: %i)...\n", millis() % req_bmm_interval);
  if (charging_switch_state == 1 && highest_cell_voltage > max_cell_voltage) {
    charging_switch_state = 0;
  } 
  else if (highest_cell_temp > max_cell_temp) {
    charging_switch_state = 0;
  }
  else if (charging_switch_state == 0 && highest_cell_voltage < max_cell_voltage_release) {
    charging_switch_state = 1;
  }
}

void set_charging_switch_state() {
  digitalWrite(charging_mosfet_pin, charging_switch_state);
}

void get_load_switch_state() {

  //Serial.printf("Getting load_switch_state (millis: %i)...\n", millis() % req_bmm_interval);
  if (load_switch_state == 1 && lowest_cell_voltage < min_cell_voltage) {
    load_switch_state = 0;
  }
  else if (highest_cell_temp > max_cell_temp) {
    load_switch_state = 0;
  }
  else if (load_switch_state == 0 && lowest_cell_voltage > min_cell_voltage_release) {
    load_switch_state = 1;
  }
}

void set_load_switch_state() {
  digitalWrite(load_mosfet_pin, load_switch_state);
}

void compute_soc_soe(float current_in /*[A]*/, float current_out /*[A]*/, float voltage /*[V]*/, float integration_interval /*[ms]*/) {

  //Serial.printf("Computing SoC and SoE (millis: %i)...\n", millis() % req_bmm_interval);
  float current = (current_in - current_out) * 1000; // current is in mA
  current = current - get_system_avg_current_consumption() * 1000;
  //Serial.printf("Battery current for SoC and SoE: %.2f mA\n", current);

  if (current > 0) {
    battery_reached_bottom = 0;
  }

  if (current > 0 && highest_cell_voltage > max_cell_voltage) {

    /* Update charge capacity when the state of charge reach the top (max_cell_voltage landmark) */
    charge_capacity_mah = state_of_charge_mah;
    /* Update energy capacity when the state of energy reach the top (max_cell_voltage landmark) */
    energy_capacity_mwh = state_of_energy_mwh;

    //Serial.println("Battery state reached the top");

  }
  else if (current < 0 && lowest_cell_voltage < min_cell_voltage) {
    
    /* Update charge capacity when the state of charge reach the bottom (min_cell_voltage landmark) */
    charge_capacity_mah = charge_capacity_mah - state_of_charge_mah;
    state_of_charge_mah = 0;
    /* Update energy capacity when the state of energy reach the bottom (min_cell_voltage landmark) */
    energy_capacity_mwh = energy_capacity_mwh - state_of_energy_mwh;
    state_of_energy_mwh = 0;

    battery_reached_bottom = 1;

    //Serial.println("Battery state reached the bottom");

  }
  else if (battery_reached_bottom == 0) {

    /* Update state of charge */
    state_of_charge_mah = state_of_charge_mah + current * (integration_interval/(1000*60*60));
    /* Update state of energy */
    state_of_energy_mwh = state_of_energy_mwh + voltage * current * (integration_interval/(1000*60*60));

    //Serial.printf("state_of_charge_mah: %.2f  current_integration: %.2f\n", state_of_charge_mah, current*(integration_interval/(1000*60*60)));

  }
  
  state_of_charge_perc = (state_of_charge_mah / charge_capacity_mah) * 100;
  state_of_energy_perc = (state_of_energy_mwh / energy_capacity_mwh) * 100;
  
  /* Keep the state of charge always between 0% and 100% */
  if (state_of_charge_perc <= 0) {
    state_of_charge_perc = 0;
  } else if (state_of_charge_perc >= 100) {
    state_of_charge_perc = 100;
  }
  /* Keep the state of energy always between 0% and 100% */
  if (state_of_energy_perc <= 0) {
    state_of_energy_perc = 0;
  } else if (state_of_energy_perc >= 100) {
    state_of_energy_perc = 100;
  }
}

float get_system_avg_current_consumption() {
  
  float bmms_current = 0;
  float bmm_0_current = 0;
  float bmm_1_current = 0;
  float bmm_2_current = 0;
  float bmu_current = 0;
  float bmms_power = 1; // [W]
  float bmu_power = map(bmm_pack_voltage, 16.2, 37.8, 0.95, 0.62); // [W]

  bmm_0_current = bmms_power / (cell_voltage[0]+cell_voltage[1]+cell_voltage[2]);
  bmm_1_current = bmms_power / (cell_voltage[3]+cell_voltage[4]+cell_voltage[5]);
  bmm_2_current = bmms_power / (cell_voltage[6]+cell_voltage[7]+cell_voltage[8]);
  bmms_current = (bmm_0_current + bmm_1_current + bmm_2_current) / 3;
  bmu_current = bmu_power / bmm_pack_voltage;

  system_avg_current_consumption = bmms_current + bmu_current;

  return system_avg_current_consumption;
}

void log_to_sd_card() {
  
  if (sd_card_initialized && !eject_sd_card) {

    //Serial.printf("Logging to SD card (millis: %i)...\n", millis() % req_bmm_interval);
    
    // File file;
    // now = rtc.now();

    /* Initialize daily data 86400 file if not existent */
    // char state_log_86400_file_name[28];
    // snprintf( state_log_86400_file_name, sizeof(state_log_86400_file_name), "/%02d%02d%02d_data_86400.txt", now.year(), now.month(), now.day()); /* /20210514_data_86400.txt */
    // file = SD.open(state_log_86400_file_name, "r");
    // if(!file || file.isDirectory()){
    //   char state_log_first_line[256] = "date_time,soe,ec,soc,cc,pack_v,load_c,chrg_c,sys_c,load_s_s,chrg_s_s,c_0_v,c_1_v,c_2_v,c_3_v,c_4_v,c_5_v,c_6_v,c_7_v,c_8_v,c_0_t,c_1_t,c_2_t,c_3_t,c_4_t,c_5_t,c_6_t,c_7_t,c_8_t,c_0_b,c_1_b,c_2_b,c_3_b,c_4_b,c_5_b,c_6_b,c_7_b,c_8_b\r\n";
    //   write_file(SD, state_log_86400_file_name, String(state_log_first_line).c_str());
    // }
    // file.close();

    /* Initialize daily data 1440 file if not existent */
    // char state_log_1440_file_name[28];
    // snprintf( state_log_1440_file_name, sizeof(state_log_1440_file_name), "/%02d%02d%02d_data_1440.txt", now.year(), now.month(), now.day()); /* /20210514_data_1440.txt */
    // file = SD.open(state_log_1440_file_name, "r");
    // if(!file || file.isDirectory()){
    //   char state_log_first_line[256] = "date_time,soe,ec,soc,cc,pack_v,load_c,chrg_c,sys_c,load_s_s,chrg_s_s,c_0_v,c_1_v,c_2_v,c_3_v,c_4_v,c_5_v,c_6_v,c_7_v,c_8_v,c_0_t,c_1_t,c_2_t,c_3_t,c_4_t,c_5_t,c_6_t,c_7_t,c_8_t,c_0_b,c_1_b,c_2_b,c_3_b,c_4_b,c_5_b,c_6_b,c_7_b,c_8_b\r\n";
    //   write_file(SD, state_log_1440_file_name, String(state_log_first_line).c_str());
    //   file.close();
    // }

    /* Increment log counter */
    sd_log_counter++;

    /* Update BMS state variables in SD card */
    write_file(SD, "/charge_capacity_mah.txt", String(charge_capacity_mah).c_str());
    write_file(SD, "/state_of_charge_mah.txt", String(state_of_charge_mah).c_str());
    write_file(SD, "/energy_capacity_mwh.txt", String(energy_capacity_mwh).c_str());
    write_file(SD, "/state_of_energy_mwh.txt", String(state_of_energy_mwh).c_str());
    write_file(SD, "/sd_log_counter.txt", String(sd_log_counter).c_str());

    /* Prepare state log message */
    // char state_log[256];
    // snprintf( state_log, sizeof(state_log), "%02d-%02d-%02d_%02d-%02d-%02d,%.2f,%.2f,%.2f,%.2f,%.3f,%.3f,%.3f,%.2f,%i,%i,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\r\n",
    //                                         now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second(),
    //                                         state_of_energy_mwh, energy_capacity_mwh, state_of_charge_mah, charge_capacity_mah,
    //                                         bmm_pack_voltage, load_current, charging_current, system_avg_current_consumption,
    //                                         load_switch_state, charging_switch_state, cell_voltage[0], cell_voltage[1],
    //                                         cell_voltage[2], cell_voltage[3], cell_voltage[4], cell_voltage[5], cell_voltage[6],
    //                                         cell_voltage[7], cell_voltage[8], cell_temp[0], cell_temp[1], cell_temp[2], cell_temp[3],
    //                                         cell_temp[4], cell_temp[5], cell_temp[6], cell_temp[7], cell_temp[8], cell_bal_temp[0],
    //                                         cell_bal_temp[1], cell_bal_temp[2], cell_bal_temp[3], cell_bal_temp[4], cell_bal_temp[5],
    //                                         cell_bal_temp[6], cell_bal_temp[7], cell_bal_temp[8] );

    /* Prepare system log message */
    // prepare a system log file
    
    /* Verify if 60 cycles has passed */
    // if (sd_log_counter == 60) {

    //   /* Append new line to daily data 86400 file */
    //   write_file(SD, state_log_86400_file_name, String(state_log).c_str());

    //   /* Append new line to daily data 1440 file */
    //   //write_file(SD, state_log_1440_file_name, String(state_log).c_str());
    //   //sd_log_counter = 0;

    // } else {

    //   /* Append new line to daily data 86400 file */
    //   write_file(SD, state_log_86400_file_name, String(state_log).c_str());
    // }

  } else {
    //Serial.printf("SD card logging is disabled\n");
  }
}

/* Write to the SD card (DON'T MODIFY THIS FUNCTION) */
void write_file(fs::FS &fs, const char * path, const char * message) {
  
  //Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    //Serial.println("File written");
  } else {
    Serial.println("Failed writting to " + String(path));
  }
  file.close();
}

/* Append data to the SD card (DON'T MODIFY THIS FUNCTION) */
void append_file(fs::FS &fs, const char * path, const char * message) {
  
  //Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    //Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

String read_file(fs::FS &fs, const char * path){
  //Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    //Serial.println("- empty file or failed to open file");
    return String();
  }
  //Serial.println("- read from file:");
  String fileContent;
  while(file.available()){
    fileContent+=String((char)file.read());
  }
  //Serial.println(fileContent);
  file.close();
  return fileContent;
}

int read_adc(int channel) {
  int value[8];
  channel = channel == 7 ? 0 : channel + 1;
  for (int i = 0; i < 8; i ++) {
    value[i] = 0;
    digitalWrite(adc_cs_pin, LOW);
    int hi = SPI.transfer( i << 3 );
    int lo = SPI.transfer( 0 );
    digitalWrite(adc_cs_pin, HIGH);
    value[i] = (hi << 8) | lo;
  }
  return value[channel];
}

void system_led() {
  if (millis() % req_bmm_interval <= 500 && sys_led_flag) {
    pixels.setPixelColor(0, pixels.Color(0, 255, 0));
    pixels.show();
    pixels.setPixelColor(0, pixels.Color(0, 255, 0));
    pixels.show();
    sys_led_flag = 0;
  } else if (millis() % req_bmm_interval > 500 && !sys_led_flag) {
    pixels.setPixelColor(0, pixels.Color(0, 0, 0));
    pixels.show();
    pixels.setPixelColor(0, pixels.Color(0, 0, 0));
    pixels.show();
    sys_led_flag = 1;
  }
}