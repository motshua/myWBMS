#include <OneWire.h> 
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4

TaskHandle_t task_ds18b20_h;

RTC_DATA_ATTR int flag_read_ds18b20 = 5;
bool flag_ds18b20_ready = 0;
bool flag_ds18b20_taken = 0;
RTC_DATA_ATTR float ds18b20_temp = 0;
long last_millis = 0;
const uint8_t ds18b20_1_addr[] = {0x28, 0x66, 0xEE, 0x2D, 0x33, 0x20, 0x01, 0x5F};
const uint8_t ds18b20_2_addr[] = {0x28, 0x40, 0x7D, 0x5C, 0x33, 0x20, 0x01, 0x85};
const uint8_t ds18b20_3_addr[] = {0x28, 0x9E, 0x8C, 0x51, 0x33, 0x20, 0x01, 0xA4};
const uint8_t ds18b20_4_addr[] = {0x28, 0x20, 0x34, 0x48, 0x33, 0x20, 0x01, 0x6A};
const uint8_t ds18b20_5_addr[] = {0x28, 0xEE, 0xB1, 0xBE, 0x33, 0x20, 0x01, 0x87};
const uint8_t ds18b20_6_addr[] = {0x28, 0xEE, 0xB1, 0xBE, 0x33, 0x20, 0x01, 0x87};

void code_for_task_ds18b20( void * parameter ) {

  OneWire oneWire(ONE_WIRE_BUS);
  DallasTemperature ds18b20_sensors(&oneWire);
  int numberOfDevices;
  DeviceAddress tempDeviceAddress;

  Serial.println("Starting ds18b20 sensors... (" + String(millis()) + " ms on core 0)");
  ds18b20_sensors.begin();
  
  //numberOfDevices = ds18b20_sensors.getDeviceCount();
  //Serial.print(numberOfDevices, DEC);
  //Serial.println(" devices.");
  
  /* To get sensors address use: */
  //for(int i=0; i<numberOfDevices; i++) {
  //  if(ds18b20_sensors.getAddress(tempDeviceAddress, i)) {
  //    Serial.print("Found device ");
  //    Serial.print(i, DEC);
  //    Serial.print(" with address: ");
  //    for (uint8_t i = 0; i < 8; i++) {
  //      if (tempDeviceAddress[i] < 16) 
  //        Serial.print("0");
  //        Serial.print(tempDeviceAddress[i], HEX);
  //    }
  //    Serial.println();
  //  } else {
  //    Serial.print("Found ghost device at ");
  //    Serial.print(i, DEC);
  //    Serial.print(" but could not detect address. Check power and cabling");
  //  }
  //}
  
  Serial.println("Setting ds18b20 sensors resolution... (" + String(millis()) + " ms on core 0)");
  ds18b20_sensors.setResolution(9);
  Serial.println("Requesting temp... (" + String(millis()) + " ms on core 0)");
  ds18b20_sensors.requestTemperatures();
  
  BMM_ID = preferences.getUInt("BMM_ID", 0);
  if (BMM_ID == 0) {
    ds18b20_temp = ds18b20_sensors.getTempC(ds18b20_1_addr);
    ds18b20_temp = ds18b20_temp + ds18b20_sensors.getTempC(ds18b20_2_addr);
    ds18b20_temp = ds18b20_temp/2;
  } else if (BMM_ID == 1) {
    ds18b20_temp = ds18b20_sensors.getTempC(ds18b20_3_addr);
    ds18b20_temp = ds18b20_temp + ds18b20_sensors.getTempC(ds18b20_4_addr);
    ds18b20_temp = ds18b20_temp/2;
  } else if (BMM_ID == 2) {
    ds18b20_temp = ds18b20_sensors.getTempC(ds18b20_5_addr);
    ds18b20_temp = ds18b20_temp + ds18b20_sensors.getTempC(ds18b20_6_addr);
    ds18b20_temp = ds18b20_temp/2;
  }

  Serial.println("Temperature from ds18b20 sensors: " + String(ds18b20_temp) + " C");
  
  Serial.println("Temp reading complete (" + String(millis()) + " ms on core 0)");
  flag_ds18b20_ready = 1;
  while (1) {
    delay(100); 
  }
}
