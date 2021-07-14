#include "esp_now.h"
#include "deep_sleep.h"
#include "bms.h"
#include "ds18b20.h"
 
void setup() {
  
  Serial.begin(115200);

  print_wakeup_reason();

  if (flag_read_ds18b20 == 5) {
    xTaskCreatePinnedToCore(
        code_for_task_ds18b20, // Task function
        "task_ds18b20",        // Name of task
        2000,                  // Stack size of task
        NULL,                  // Parameter of the task
        1,                     // Priority of task
        &task_ds18b20_h,       // Task handle to keep track of created task
        0);                    // Core
  }

  /* Open Preferences with my-app namespace */
  preferences.begin("bms", false);

  /* ESP32 pins setup */
  gpio_hold_dis((gpio_num_t)bal_mosfet_cell_1_pin);
  pinMode(bal_mosfet_cell_1_pin, OUTPUT);
  gpio_hold_dis((gpio_num_t)bal_mosfet_cell_2_pin);
  pinMode(bal_mosfet_cell_2_pin, OUTPUT);
  gpio_hold_dis((gpio_num_t)bal_mosfet_cell_3_pin);
  pinMode(bal_mosfet_cell_3_pin, OUTPUT);
  gpio_hold_dis((gpio_num_t)ldo_5v_en_pin);
  pinMode(ldo_5v_en_pin, OUTPUT);
  pinMode(adc_cs_pin, OUTPUT);
  pinMode(button_a_pin, INPUT);
  pinMode(button_b_pin, INPUT);
  pinMode(button_x_pin, INPUT);
  pinMode(button_y_pin, INPUT);
  set_bal_mosfets(0, 0, 0); // Setting all mosfets LOW

  /* Setup of BMM_ID value */
  if (digitalRead(button_a_pin) == LOW) {
    /* WS2812 setup */
    pixels.begin();
    pixels.clear();
    pixels.setBrightness(50); // 0 to 255
    BMM_ID = preferences.getUInt("BMM_ID", 0);
    Serial.printf("Setting up BMM_ID... Current value: %i\n", BMM_ID);
    long last_check = 0;
    int bounce_interval = 2000;
    while(1) {
      if (millis()-last_check > bounce_interval) {
        last_check = millis();
        if (digitalRead(button_y_pin) == LOW) {
          BMM_ID = BMM_ID == 2 ? 0 : BMM_ID + 1;
          Serial.printf("Current value changed to: %i\n", BMM_ID);
        } else if (digitalRead(button_b_pin) == LOW) {
          preferences.putUInt("BMM_ID", BMM_ID);
          Serial.printf("BMM_ID set to %i\n", BMM_ID);
          pixels.setPixelColor(0, pixels.Color(0, 0, 0));
          pixels.show();
          delay(10);
          pixels.setPixelColor(0, pixels.Color(0, 0, 0));
          pixels.show();
          ESP.restart();
        }
      }
      if (BMM_ID == 0) {
        pixels.setPixelColor(0, pixels.Color(255, 0, 255));
        pixels.show();
      } else if (BMM_ID == 1) {
        pixels.setPixelColor(0, pixels.Color(255, 255, 255));
        pixels.show();
      } else if (BMM_ID == 2) {
        pixels.setPixelColor(0, pixels.Color(0, 255, 255));
        pixels.show();
      }
    }
  }

  /* Get the config values from NVS, if the key does not exist, return a default value of 0 */
  BMM_ID = preferences.getUInt("BMM_ID", 0);
  //Serial.println("BMM_ID: " + String(BMM_ID));
  max_v = preferences.getFloat("max_v", 0.0);
  //Serial.println("max_v: " + String(max_v));
  min_v = preferences.getFloat("min_v", 0.0);
  //Serial.println("min_v: " + String(min_v));
  max_b_t = preferences.getFloat("max_b_t", 0.0);
  //Serial.println("max_b_t: " + String(max_b_t));

  /* SPI setup for ADC128S102 */
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE3);  // Mode=2 CPOL=1, CPHA=0

  /* Setup all the data to be send */
  Serial.println("DS18B20 flag: " + String(flag_read_ds18b20));
  bmm_out.id = BMM_ID;

  digitalWrite(ldo_5v_en_pin, LOW); // Enable 5V LDO
  delay(5); // Wait for the circuits to turn on

  get_cell_voltages();
  bmm_out.c1_v = cell_voltages[0];
  bmm_out.c2_v = cell_voltages[1];
  bmm_out.c3_v = cell_voltages[2];
  Serial.println("bmm_out.c1_v: " + String(bmm_out.c1_v) + " V");
  Serial.println("bmm_out.c2_v: " + String(bmm_out.c2_v) + " V");
  Serial.println("bmm_out.c3_v: " + String(bmm_out.c3_v) + " V");
  if (flag_read_ds18b20 == 5) {
    Serial.println("Waiting for ds18b20 sensors readings...");
    while(flag_ds18b20_taken != 1) {
      if (flag_ds18b20_ready == 1) {
        bmm_out.c1_t = ds18b20_temp;
        bmm_out.c2_t = ds18b20_temp;
        bmm_out.c3_t = ds18b20_temp;
        flag_ds18b20_taken = 1;
      }
      delay(1);
    }
  } else {
    bmm_out.c1_t = ds18b20_temp;
    bmm_out.c2_t = ds18b20_temp;
    bmm_out.c3_t = ds18b20_temp;
  }
  // Serial.println("bmm_out.c1_t: " + String(bmm_out.c1_t) + " C");
  // Serial.println("bmm_out.c2_t: " + String(bmm_out.c2_t) + " C");
  // Serial.println("bmm_out.c3_t: " + String(bmm_out.c3_t) + " C");
  get_bal_temp();
  bmm_out.c1_b_t = bal_temp[0];
  bmm_out.c2_b_t = bal_temp[1];
  bmm_out.c3_b_t = bal_temp[2];
  Serial.println("bmm_out.c1_b_t: " + String(bmm_out.c1_b_t) + " C");
  Serial.println("bmm_out.c2_b_t: " + String(bmm_out.c2_b_t) + " C");
  Serial.println("bmm_out.c3_b_t: " + String(bmm_out.c3_b_t) + " C");

  digitalWrite(ldo_5v_en_pin, HIGH); // Disable 5V LDO

  get_bal_mosfets();
  bmm_out.c1_b_p = bal_mosfet[0]*100;
  bmm_out.c2_b_p = bal_mosfet[1]*100;
  bmm_out.c3_b_p = bal_mosfet[2]*100;
  Serial.println("bmm_out.c1_b_p: " + String(bmm_out.c1_b_p) + "%");
  Serial.println("bmm_out.c2_b_p: " + String(bmm_out.c2_b_p) + "%");
  Serial.println("bmm_out.c3_b_p: " + String(bmm_out.c3_b_p) + "%");
  bmm_out.err = "NULL";
  //Serial.println("bmm_out.err: NULL");
  bmm_out.msg_count = msg_count;
  //Serial.println("bmm_out.msg_count: " + String(bmm_out.msg_count));

  /* Turn on/off balancing mosfets */
  //set_bal_mosfets(bal_mosfet[0], bal_mosfet[1], bal_mosfet[2]);
 
  /* Set device as a Wi-Fi Station */
  WiFi.mode(WIFI_STA);

  if (flag_wifi_ch == 0) {
    Serial.println("Getting Wi-Fi Channel... (current millis: " + String(millis()) + " ms)");
    channel = getWiFiChannel(mySSID);
    Serial.println("Wi-Fi Channel: " + String(channel) + " (current millis: " + String(millis()) + " ms)");
    flag_wifi_ch = 1;
  }

  //WiFi.printDiag(Serial); // Uncomment to verify channel number before
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE); // Needs to be the same as in the BMU
  esp_wifi_set_promiscuous(false);
  //WiFi.printDiag(Serial); // Uncomment to verify channel change after

  /* Init ESP-NOW */
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  /* Once ESPNow is successfully init, register send callback */
  esp_now_register_send_cb(OnDataSent);

  /* Register for a callback function that will be called when data is received */
  esp_now_register_recv_cb(OnDataRecv);
  
  /* Register peer */
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, bmu_address, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  /* Add peer */   
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    ESP.restart();
    return;
  }

  Serial.println("Ready to receive (current millis: " + String(millis()) + " ms)");
  int t_ready_to_receive = millis();
  const int t_sleep_without_cell_temp = 610; 
  const int t_sleep_with_cell_temp = 250;
  const int t_receive_timeout = 1999;
  int t_sleep;
  
  do {
    
    if ( new_msg == 1 ) {

      Serial.print("Sending msg... (current millis: " + String(millis()) + ") ");

      bmm_out.time_waiting = millis() - t_ready_to_receive;
      bmm_out.time_awake = millis() + 20;
  
      // Send message via ESP-NOW
      esp_err_t result = esp_now_send(bmu_address, (uint8_t *) &bmm_out, sizeof(bmm_out));
      
      if (result == ESP_OK) {
        Serial.print("sent with success, current millis: ");
        Serial.println(millis());
      }
      else {
        Serial.print("error sending the data, current millis: ");
        Serial.println(millis());
      }

      // Compare config values from BMU to current values on NVS
      // If some value has changed, update the NVS
      if (max_v != bmu_in.max_v) {
        preferences.putFloat("max_v", bmu_in.max_v);
        Serial.println("Max_v: " + String(max_v) + " -> " + String(bmu_in.max_v));
      }
      if (min_v != bmu_in.min_v) {
        preferences.putFloat("min_v", bmu_in.min_v);
        Serial.println("Min_v: " + String(min_v) + " -> " + String(bmu_in.min_v));
      }
      if (max_b_t != bmu_in.max_b_t) {
        preferences.putFloat("max_b_t", bmu_in.max_b_t);
        Serial.println("Max_b_t: " + String(max_b_t) + " -> " + String(bmu_in.max_b_t));
      }

      chrg_state = bmu_in.chrg_state;
      lowest_v = bmu_in.lowest_v;
      
      // Print config values in the Serial
      Serial.println("Max_v: " + String(bmu_in.max_v) + " V");
      Serial.println("Min_v: " + String(bmu_in.min_v) + " V");
      Serial.println("Max_bal_temp: " + String(bmu_in.max_b_t) + " C");
      Serial.println("Sync: " + String(bmu_in.sync) + " ms");
      Serial.println("Message_count: " + String(bmu_in.msg_count));
      
      msg_count++;
      new_msg = 2;
    }
  } while ((new_msg != 2) && (millis() < t_receive_timeout));

  flag_read_ds18b20--;

  // Calcular o tempo de deep sleep usando o tempo em que a placa permanceceu acordada
      
  if (flag_read_ds18b20 == 0) { // will wake up early to use ds18b20
    t_sleep = (t_sleep_with_cell_temp - bmu_in.sync) * 1000; // (in micro seconds)
    flag_read_ds18b20 = 5;
  } else {
    t_sleep = (t_sleep_without_cell_temp - bmu_in.sync) * 1000; // (in micro seconds)
  }

  if (millis() >= t_receive_timeout) {
    Serial.println("Receive timeout");
    t_sleep = 1000; // (in micro seconds)
  }

  esp_sleep_enable_timer_wakeup( t_sleep ); // (in micro seconds)
  Serial.println("Setting ESP32 to sleep for " + String(t_sleep/1000) +
  " milliseconds...");
  
  gpio_hold_en((gpio_num_t)bal_mosfet_cell_1_pin);
  gpio_hold_en((gpio_num_t)bal_mosfet_cell_2_pin);
  gpio_hold_en((gpio_num_t)bal_mosfet_cell_3_pin);
  gpio_hold_en((gpio_num_t)ldo_5v_en_pin);
  gpio_deep_sleep_hold_en();
  Serial.println("Going to sleep now... (current millis: " + String(millis()) + ")");
  Serial.println("\n\n");
  Serial.flush(); 
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

void loop() {
  //This is not going to be called
}
