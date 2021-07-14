/* Local files */
#include "esp_now.h"

void setup() {
  
  Serial.begin(115200);

  Serial.printf("\n\n");
  delay(5000);

  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  /* Verify if all the settings exists on SPIFFS, and if not, create with standard values */
  File file;
  file = SPIFFS.open("/min_cell_voltage.txt", "r");
  if(!file || file.isDirectory()){
    Serial.println("Setting min_cell_voltage to standard value...");
    write_file(SPIFFS, "/min_cell_voltage.txt", min_cell_voltage_std.c_str());
    file.close();
  }
  file = SPIFFS.open("/min_cell_voltage_release.txt", "r");
  if(!file || file.isDirectory()){
    write_file(SPIFFS, "/min_cell_voltage_release.txt", min_cell_voltage_release_std.c_str());
    file.close();
  }
  file = SPIFFS.open("/max_cell_voltage.txt", "r");
  if(!file || file.isDirectory()){
    write_file(SPIFFS, "/max_cell_voltage.txt", max_cell_voltage_std.c_str());
    file.close();
  }
  file = SPIFFS.open("/max_cell_voltage_release.txt", "r");
  if(!file || file.isDirectory()){
    write_file(SPIFFS, "/max_cell_voltage_release.txt", max_cell_voltage_release_std.c_str());
    file.close();
  }
  file = SPIFFS.open("/max_load_current.txt", "r");
  if(!file || file.isDirectory()){
    write_file(SPIFFS, "/max_load_current.txt", max_load_current_std.c_str());
    file.close();
  }
  file = SPIFFS.open("/max_charging_current.txt", "r");
  if(!file || file.isDirectory()){
    write_file(SPIFFS, "/max_charging_current.txt", max_charging_current_std.c_str());
    file.close();
  }
  file = SPIFFS.open("/min_cell_temp.txt", "r");
  if(!file || file.isDirectory()){
    write_file(SPIFFS, "/min_cell_temp.txt", min_cell_temp_std.c_str());
    file.close();
  }
  file = SPIFFS.open("/max_cell_temp.txt", "r");
  if(!file || file.isDirectory()){
    write_file(SPIFFS, "/max_cell_temp.txt", max_cell_temp_std.c_str());
    file.close();
  }
  file = SPIFFS.open("/max_cell_bal_temp.txt", "r");
  if(!file || file.isDirectory()){
    write_file(SPIFFS, "/max_cell_bal_temp.txt", max_cell_bal_temp_std.c_str());
    file.close();
  }
  file = SPIFFS.open("/eject_sd_card.txt", "r");
  if(!file || file.isDirectory()){
    write_file(SPIFFS, "/eject_sd_card.txt", eject_sd_card_std.c_str());
    file.close();
  }
  file = SPIFFS.open("/lang.txt", "r");
  if(!file || file.isDirectory()){
    write_file(SPIFFS, "/lang.txt", lang_std.c_str());
    file.close();
  }

  /* Update BMS variables using values from SPIFFS */
  Serial.println("Updating BMS variables using values from SPIFFS...");
  min_cell_voltage = read_file(SPIFFS, "/min_cell_voltage.txt").toFloat();
  min_cell_voltage_release = read_file(SPIFFS, "/min_cell_voltage_release.txt").toFloat();
  max_cell_voltage = read_file(SPIFFS, "/max_cell_voltage.txt").toFloat();
  max_cell_voltage_release = read_file(SPIFFS, "/max_cell_voltage_release.txt").toFloat();
  max_load_current = read_file(SPIFFS, "/max_load_current.txt").toFloat();
  max_charging_current = read_file(SPIFFS, "/max_charging_current.txt").toFloat();
  min_cell_temp = read_file(SPIFFS, "/min_cell_temp.txt").toFloat();
  max_cell_temp = read_file(SPIFFS, "/max_cell_temp.txt").toFloat();
  max_cell_bal_temp = read_file(SPIFFS, "/max_cell_bal_temp.txt").toFloat();
  eject_sd_card = read_file(SPIFFS, "/eject_sd_card.txt") == "no" ? 0 : 1;

  if (eject_sd_card) {
    Serial.printf("SD card logging is disabled\n");
  }

  /* Set the device as a Station and Soft Access Point simultaneously */
  WiFi.mode(WIFI_AP_STA);

  /* Set device as a Wi-Fi Station */
  WiFi.begin(mySSID, myPASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Setting as a Wi-Fi Station..");
  }
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());

  /* Init ESP-NOW */
  if (esp_now_init() != ESP_OK) {
   Serial.println("Error initializing ESP-NOW");
   return;
  }

  /* Once ESPNow is successfully Init, we will register for Send CB to get the status of Trasnmitted packet */
  esp_now_register_send_cb(OnDataSent);
  Serial.println("ESP-NOW send callback registered");
  
  /* Register for a callback function that will be called when data is received */
  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("ESP-NOW receive callback registered");

  /* Register peer */
  esp_now_peer_info_t peerInfo;
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  /* Register bmm_0 peer */  
  memcpy(peerInfo.peer_addr, bmm_0_address, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  /* Register bmm_1 peer */  
  memcpy(peerInfo.peer_addr, bmm_1_address, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  /* Register bmm_2 peer */  
  memcpy(peerInfo.peer_addr, bmm_2_address, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  /* Send web page (dashboard) to client */
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
    Serial.println("Dashboard sent to client");
  });

  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });

  /* Send a GET request to <ESP_IP>/get?inputString=<inputMessage> */
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {

    Serial.printf("GET request received\n");

    // GET inputString value on <ESP_IP>/get?inputString=<inputMessage>
    if (request->hasParam(PARAM_MIN_CELL_VOLTAGE)) {
      String min_cell_voltage_config = request->getParam(PARAM_MIN_CELL_VOLTAGE)->value();
      Serial.printf("Updating min_cell_voltage_config to %s\n", min_cell_voltage_config);
      write_file(SPIFFS, "/min_cell_voltage.txt", min_cell_voltage_config.c_str());
      min_cell_voltage = min_cell_voltage_config.toFloat();
    }
    if (request->hasParam(PARAM_MIN_CELL_VOLTAGE_RELEASE)) {
      String min_cell_voltage_release_config = request->getParam(PARAM_MIN_CELL_VOLTAGE_RELEASE)->value();
      Serial.printf("Updating min_cell_voltage_release_config to %s\n", min_cell_voltage_release_config);
      write_file(SPIFFS, "/min_cell_voltage_release.txt", min_cell_voltage_release_config.c_str());
      min_cell_voltage_release = min_cell_voltage_release_config.toFloat();
    }
    if (request->hasParam(PARAM_MAX_CELL_VOLTAGE)) {
      String max_cell_voltage_config = request->getParam(PARAM_MAX_CELL_VOLTAGE)->value();
      Serial.printf("Updating max_cell_voltage_config to %s\n", max_cell_voltage_config);
      write_file(SPIFFS, "/max_cell_voltage.txt", max_cell_voltage_config.c_str());
      max_cell_voltage = max_cell_voltage_config.toFloat();
    }
    if (request->hasParam(PARAM_MAX_CELL_VOLTAGE_RELEASE)) {
      String max_cell_voltage_release_config = request->getParam(PARAM_MAX_CELL_VOLTAGE_RELEASE)->value();
      Serial.printf("Updating max_cell_voltage_release_config to %s\n", max_cell_voltage_release_config);
      write_file(SPIFFS, "/max_cell_voltage_release.txt", max_cell_voltage_release_config.c_str());
      max_cell_voltage_release = max_cell_voltage_release_config.toFloat();
    }
    if (request->hasParam(PARAM_MAX_LOAD_CURRENT)) {
      String max_load_current_config = request->getParam(PARAM_MAX_LOAD_CURRENT)->value();
      Serial.printf("Updating max_load_current_config to %s\n", max_load_current_config);
      write_file(SPIFFS, "/max_load_current.txt", max_load_current_config.c_str());
      max_load_current = max_load_current_config.toFloat();
    }
    if (request->hasParam(PARAM_MAX_CHARGING_CURRENT)) {
      String max_charging_current_config = request->getParam(PARAM_MAX_CHARGING_CURRENT)->value();
      Serial.printf("Updating max_charging_current_config to %s\n", max_charging_current_config);
      write_file(SPIFFS, "/max_charging_current.txt", max_charging_current_config.c_str());
      max_charging_current = max_charging_current_config.toFloat();
    }
    if (request->hasParam(PARAM_MIN_CELL_TEMP)) {
      String min_cell_temp_config = request->getParam(PARAM_MIN_CELL_TEMP)->value();
      Serial.printf("Updating min_cell_temp_config to %s\n", min_cell_temp_config);
      write_file(SPIFFS, "/min_cell_temp.txt", min_cell_temp_config.c_str());
      min_cell_temp = min_cell_temp_config.toFloat();
    }
    if (request->hasParam(PARAM_MAX_CELL_TEMP)) {
      String max_cell_temp_config = request->getParam(PARAM_MAX_CELL_TEMP)->value();
      Serial.printf("Updating max_cell_temp_config to %s\n", max_cell_temp_config);
      write_file(SPIFFS, "/max_cell_temp.txt", max_cell_temp_config.c_str());
      max_cell_temp = max_cell_temp_config.toFloat();
    }
    if (request->hasParam(PARAM_MAX_CELL_BAL_TEMP)) {
      String max_cell_bal_temp_config = request->getParam(PARAM_MAX_CELL_BAL_TEMP)->value();
      Serial.printf("Updating max_cell_bal_temp_config to %s\n", max_cell_bal_temp_config);
      write_file(SPIFFS, "/max_cell_bal_temp.txt", max_cell_bal_temp_config.c_str());
      max_cell_bal_temp = max_cell_bal_temp_config.toFloat();
    }
    if (request->hasParam(PARAM_EJECT_SD_CARD)) {
      String eject_sd_card_config = request->getParam(PARAM_EJECT_SD_CARD)->value();
      Serial.printf("Updating eject_sd_card_config...\n");
      write_file(SPIFFS, "/eject_sd_card.txt", eject_sd_card_config.c_str());
      eject_sd_card = eject_sd_card_config == "no" ? 0 : 1;
      if (eject_sd_card) {
        Serial.printf("SD card logging is now disabled\n");
      } else {
        Serial.printf("SD card logging is now enabled\n");
      }
    }
    if (request->hasParam(PARAM_LANG)) {
      String lang_config = request->getParam(PARAM_LANG)->value();
      Serial.printf("Updating lang_config to %s\n", lang_config);
      write_file(SPIFFS, "/lang.txt", lang_config.c_str());
    }
    if (request->hasParam(PARAM_CHARGE_CAPACITY)) {
      String charge_capacity_config = request->getParam(PARAM_CHARGE_CAPACITY)->value();
      Serial.printf("Updating charge_capacity_config to %s\n", charge_capacity_config);
      charge_capacity_mah = charge_capacity_config.toFloat();
    }
    if (request->hasParam(PARAM_ENERGY_CAPACITY)) {
      String energy_capacity_config = request->getParam(PARAM_ENERGY_CAPACITY)->value();
      Serial.printf("Updating energy_capacity_config to %s\n", energy_capacity_config);
      energy_capacity_mwh = energy_capacity_config.toFloat();
    }
  });
  
  server.addHandler(&events);
  server.onNotFound(notFound);
  server.begin();
  Serial.println("Server started");

  delay(1000);

  /* ESP32 pins setup */
  pinMode(charging_mosfet_pin, OUTPUT);
  pinMode(load_mosfet_pin, OUTPUT);
  pinMode(adc_cs_pin, OUTPUT);
  pinMode(sd_cs_pin, OUTPUT);

  /* Initialize RTC chip */
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    /* Initialize a NTPClient to get time */
    time_client.begin();
    /* Set offset time in seconds to adjust for your timezone, for example:
    * GMT +1 = 3600
    * GMT 0 = 0
    * GMT -3 = -10800 */
    time_client.setTimeOffset(-10800);
    while(!time_client.update()) {
      time_client.forceUpdate();
    }
    /* The formattedDate comes with the following format: 2018-05-28T16:00:13Z */
    formatted_date = time_client.getFormattedDate();
    /* Extract date */
    date_year = formatted_date.substring(0, 4).toInt();
    date_month = formatted_date.substring(5, 7).toInt();
    date_day = formatted_date.substring(8, 10).toInt();
    Serial.print(date_day);
    Serial.print("/");
    Serial.print(date_month);
    Serial.print("/");
    Serial.println(date_year);
    /* Extract time */
    time_hour = formatted_date.substring(11, 13).toInt();
    time_minute = formatted_date.substring(14, 16).toInt();
    time_second = formatted_date.substring(17, 19).toInt();
    Serial.print(time_hour);
    Serial.print(":");
    Serial.print(time_minute);
    Serial.print(":");
    Serial.println(time_second);  
    rtc.adjust(DateTime(date_year, date_month, date_day, time_hour, time_minute, time_second));
  }
  now = rtc.now();
  Serial.print(now.day(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.year(), DEC);
  Serial.print(" (");
  Serial.print(days_of_the_week[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  /* Initialize SD card */
  SD.begin(sd_cs_pin);  
  if(!SD.begin(sd_cs_pin)) {
    Serial.println("Card Mount Failed");
    //return;
  }
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    //return;
  }
  Serial.println("Initializing SD card...");
  if (!SD.begin(sd_cs_pin)) {
    Serial.println("ERROR - SD card initialization failed!");
    sd_card_initialized = 0;
    //return;    // init failed
  }

  if (sd_card_initialized) {
    
    /* Verify if all the BMS files exists on SD, and if not, create with initial values */
    file = SD.open("/charge_capacity_mah.txt", "r");
    if(!file || file.isDirectory()){
      write_file(SD, "/charge_capacity_mah.txt", String(charge_capacity_mah).c_str());
      file.close();
    }
    file = SD.open("/state_of_charge_mah.txt", "r");
    if(!file || file.isDirectory()){
      write_file(SD, "/state_of_charge_mah.txt", String(state_of_charge_mah).c_str());
      file.close();
    }
    file = SD.open("/energy_capacity_mwh.txt", "r");
    if(!file || file.isDirectory()){
      write_file(SD, "/energy_capacity_mwh.txt", String(energy_capacity_mwh).c_str());
      file.close();
    }
    file = SD.open("/state_of_energy_mwh.txt", "r");
    if(!file || file.isDirectory()){
      write_file(SD, "/state_of_energy_mwh.txt", String(state_of_energy_mwh).c_str());
      file.close();
    }

    /* Update BMS state variables using values from SD */
    Serial.println("Updating BMS variables using values from SD card...");
    charge_capacity_mah = read_file(SD, "/charge_capacity_mah.txt").toFloat();
    state_of_charge_mah = read_file(SD, "/state_of_charge_mah.txt").toFloat();
    energy_capacity_mwh = read_file(SD, "/energy_capacity_mwh.txt").toFloat();
    state_of_energy_mwh = read_file(SD, "/state_of_energy_mwh.txt").toFloat();
    Serial.printf("Charge capacity: %.2f mAh\n", charge_capacity_mah);
    Serial.printf("State of charge: %.2f mAh\n", state_of_charge_mah);
    Serial.printf("Energy capacity: %.2f mWh\n", energy_capacity_mwh);
    Serial.printf("State of energy: %.2f mWh\n", state_of_energy_mwh);
  }

  /* SPI setup for ADC128S102 */
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE3);  // Mode=2 CPOL=1, CPHA=0

  /* WS2812 */
  pixels.begin();
  pixels.clear();
  pixels.setBrightness(50); // 0 to 255
  pixels.setPixelColor(0, pixels.Color(0, 255, 0));
  pixels.show();
  pixels.setPixelColor(0, pixels.Color(0, 255, 0));
  pixels.show();

  /* Current sensors offset calibration */
  calibrate_current_sensors();

  /* Set cell voltages to zero */
  memset(cell_voltage, 0, sizeof(cell_voltage));

  /* Set cell temp to zero */
  memset(cell_temp, 0, sizeof(cell_temp));
}

void loop() {
  
  /* Prepare BMU message to be send to the modules */
  prepare_bmu_msg();

  /* Send request message to the modules */
  send_msg_to_bmms();

  /* If any of the modules failed to send a message back, request again */
  //request_again();

  /* Save bmm data as soon as they arrive */
  save_bmm_data();
  
  /* Update bmm data on dashboard */
  update_bmm_data_on_dashboard();

  /* Compute bmu data */
  compute_bmu_data();

  /* Update bmu data on dashboard */
  update_bmu_data_on_dashboard();

  /* Update system LED */
  system_led();

  /* Reset all flags */
  reset_flags();
  
}
