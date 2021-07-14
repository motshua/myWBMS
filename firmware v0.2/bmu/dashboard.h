/* External libraries */
#include <Arduino.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include "ESPAsyncWebServer.h"
#include <Arduino_JSON.h>

const String min_cell_voltage_std         = "3.00";
const String min_cell_voltage_release_std = "3.40";
const String max_cell_voltage_std         = "4.10";
const String max_cell_voltage_release_std = "4.00";
const String max_load_current_std         = "2.00";
const String max_charging_current_std     = "1.00";
const String min_cell_temp_std            = "5.00";
const String max_cell_temp_std            = "38.00";
const String max_cell_bal_temp_std        = "60.00";
const String eject_sd_card_std            = "no";
const String lang_std                     = "pt_br";

JSONVar json_bmm_0;
JSONVar json_bmm_1;
JSONVar json_bmm_2;
JSONVar json_bmu;
JSONVar json_lang;

const unsigned int update_dashboard_interval = 1000;

AsyncWebServer server(80);
AsyncEventSource events("/events");

const char* PARAM_MIN_CELL_VOLTAGE         = "min_cell_voltage_config";
const char* PARAM_MIN_CELL_VOLTAGE_RELEASE = "min_cell_voltage_release_config";
const char* PARAM_MAX_CELL_VOLTAGE         = "max_cell_voltage_config";
const char* PARAM_MAX_CELL_VOLTAGE_RELEASE = "max_cell_voltage_release_config";
const char* PARAM_MAX_LOAD_CURRENT         = "max_load_current_config";
const char* PARAM_MAX_CHARGING_CURRENT     = "max_charging_current_config";
const char* PARAM_MIN_CELL_TEMP            = "min_cell_temp_config";
const char* PARAM_MAX_CELL_TEMP            = "max_cell_temp_config";
const char* PARAM_MAX_CELL_BAL_TEMP        = "max_cell_bal_temp_config";
const char* PARAM_EJECT_SD_CARD            = "eject_sd_card_config";
const char* PARAM_LANG                     = "lang_config";
const char* PARAM_CHARGE_CAPACITY          = "charge_capacity_config";
const char* PARAM_ENERGY_CAPACITY          = "energy_capacity_config";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

