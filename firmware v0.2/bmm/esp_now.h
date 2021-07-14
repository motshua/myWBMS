#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <credentials.h>

// Network credentials
const char* ssid     = mySSID;
const char* password = myPASSWORD;

uint8_t bmu_address[] = {0xf0, 0x08, 0xd1, 0xc8, 0x76, 0xf8};

RTC_DATA_ATTR long msg_count = 0;
int new_msg = 0;

RTC_DATA_ATTR bool flag_wifi_ch = 0;
RTC_DATA_ATTR int32_t channel;

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

struct_bmu bmu_in;
struct_bmm bmm_out;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.print(status == ESP_NOW_SEND_SUCCESS ? "Delivery success, millis: " : "Delivery fail, millis: ");
  Serial.println(millis());
  Serial.println("");
  if (status == 0){
    //success = "Delivery success";
  } else {
    //success = "Delivery fail";
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&bmu_in, incomingData, sizeof(bmu_in));
  Serial.print("Bytes received from esp-now: ");
  Serial.print(len);
  Serial.println(", current millis: " + String(millis()) );
  new_msg = 1;
}

int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
      for (uint8_t i=0; i<n; i++) {
          if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
              return WiFi.channel(i);
          }
      }
  }
  return 0;
}
