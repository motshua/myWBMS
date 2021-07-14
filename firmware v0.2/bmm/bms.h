#include <Preferences.h> // for NVS
Preferences preferences;

#include "SPI.h" // for ADC128S102

#include <Adafruit_NeoPixel.h> // for WS2812
Adafruit_NeoPixel pixels(1, 14, NEO_GRB + NEO_KHZ800);

/* ESP32 Pins */
#define bal_mosfet_cell_1_pin 27
#define bal_mosfet_cell_2_pin 26
#define bal_mosfet_cell_3_pin 25
#define ldo_5v_en_pin 33
#define ext_temp_pin 14
#define adc_cs_pin 5
#define button_y_pin 36
#define button_x_pin 39
#define button_b_pin 34
#define button_a_pin 35

/* ADC Pins */
#define adc_test_pin 0        // IN0
#define adc_bal_temp_1_pin 1  // IN1
#define adc_bal_temp_2_pin 2  // IN2
#define adc_bal_temp_3_pin 3  // IN3
#define adc_cell_3_pin 5      // IN5
#define adc_cell_2_pin 6      // IN6
#define adc_cell_1_pin 7      // IN7

/* Functions header */
void get_cell_voltages();
int read_adc(int channel);
void get_bal_temp();
void get_adc_test_pin_value();
void set_bal_mosfets(bool bal_mosfet_0, bool bal_mosfet_1, bool bal_mosfet_2);
void get_bal_mosfets();

unsigned int BMM_ID;
float max_v; 
float min_v;
float max_b_t;
RTC_DATA_ATTR bool chrg_state = 0;
RTC_DATA_ATTR float lowest_v = 0;

float cell_voltages[] = {0, 0, 0};
bool bal_mosfet[] = {0, 0, 0};
float bal_temp[] = {0, 0, 0};

void get_cell_voltages() {
  /* Array to store the cell voltages */
  float cell_voltages_buff[] = {0, 0, 0};
  /* Number of samples to get the average */
  const uint8_t sample_size = 10;
  for (int i = 0; i < sample_size; i++) {
    cell_voltages_buff[0] += read_adc(adc_cell_1_pin);
    cell_voltages_buff[1] += read_adc(adc_cell_2_pin);
    cell_voltages_buff[2] += read_adc(adc_cell_3_pin);
    delayMicroseconds(100);
  }
  for (int i = 0; i < 3; i++) {
    cell_voltages_buff[i] = map(cell_voltages_buff[i]/sample_size, 0, 4095, 0, 5000);
    //Serial.printf("cell %i: %.2f", i, cell_voltages_buff[i]);
  }
  cell_voltages_buff[0] = (((cell_voltages_buff[0]/1000)/1.09)-2.54)*23/3.97;
  cell_voltages_buff[1] = ((cell_voltages_buff[1]/1000)-2.54)*23/3.97;
  cell_voltages_buff[2] = ((cell_voltages_buff[2]/1000)-2.54)*23/3.97;
  /* Update the global variables */
  cell_voltages[0] = cell_voltages_buff[0];
  cell_voltages[1] = cell_voltages_buff[1];
  cell_voltages[2] = cell_voltages_buff[2];
  //Serial.println("Cell voltages are updated");
}

int read_adc(int channel) {
  int value[8];
  channel = channel == 7 ? 0 : channel + 1;
  for (int i = 0; i < 8; i ++) {
    value[i] = 0;
    digitalWrite(adc_cs_pin, LOW);
    int hi = SPI.transfer(i<<3);
    int lo = SPI.transfer(0);
    digitalWrite(adc_cs_pin, HIGH);
    value[i] = (hi << 8) | lo;
  }
  return value[channel];
}

void get_bal_temp() {
  /* Array to store the balancing temperatures */
  int bal_temp_buff[] = {0, 0, 0};
  /* Number of samples to get the average */
  const uint8_t sample_size = 10;
  for (int i = 0; i < sample_size; i++) {
    bal_temp_buff[0] += read_adc(adc_bal_temp_1_pin);
    bal_temp_buff[1] += read_adc(adc_bal_temp_2_pin);
    bal_temp_buff[2] += read_adc(adc_bal_temp_3_pin);
    delayMicroseconds(100);
  }
  for (int i = 0; i < 3; i++) {
    bal_temp_buff[i] = map(bal_temp_buff[i]/sample_size, 0, 4095, 0, 5000);
    //Serial.println(bal_temp_buff[i]);
  }
  /* Calc the resistence of the NTC */
  // v_ntc = v_cc*(r_ntc)/(r_ntc+r1)
  // r_ntc = (v_ntc*r1)/(v_cc-v_ntc)
  int r_ntc[] = {0, 0, 0};
  r_ntc[0] = (bal_temp_buff[0]*47000)/(5000-bal_temp_buff[0]);
  r_ntc[1] = (bal_temp_buff[1]*47000)/(5000-bal_temp_buff[1]);
  r_ntc[2] = (bal_temp_buff[2]*47000)/(5000-bal_temp_buff[2]);
  /* Convert NTC resistence to temperature */
  // From datasheet the relation resistence to temperature
  // can be approximated to:
  // temp = 302 - 25.6*ln(r_ntc)
  bal_temp[0] = (302-25.6*log(r_ntc[0]));
  bal_temp[1] = (302-25.6*log(r_ntc[1]));
  bal_temp[2] = (302-25.6*log(r_ntc[2]));
  //Serial.println("Bal temps are updated");
}

void get_adc_test_pin_value() {
  int test_pin_value = 0;
  const uint8_t sample_size = 10;
  for (int i = 0; i < sample_size; i++) {
    test_pin_value += read_adc(adc_test_pin);
    delayMicroseconds(100);
  }
  test_pin_value = map(test_pin_value/sample_size, 0, 4095, 0, 5000);
  Serial.print("Test pin value: ");
  Serial.println(test_pin_value);
}

void get_bal_mosfets() {
  const float diff_limit = 0.05;
  bool cell_1_v_over_max = (cell_voltages[0] > max_v - 0.025);
  bool cell_2_v_over_max = (cell_voltages[1] > max_v - 0.025);
  bool cell_3_v_over_max = (cell_voltages[2] > max_v - 0.025);
  bool cell_1_v_over_other = (cell_voltages[0] - lowest_v > diff_limit);
  bool cell_2_v_over_other = (cell_voltages[1] - lowest_v > diff_limit);
  bool cell_3_v_over_other = (cell_voltages[2] - lowest_v > diff_limit);
  if (chrg_state == 1) {
    bal_mosfet[0] = (( cell_1_v_over_max || cell_1_v_over_other ) && bal_temp[0] < max_b_t) ? 1 : 0;
    bal_mosfet[1] = (( cell_2_v_over_max || cell_2_v_over_other ) && bal_temp[1] < max_b_t) ? 1 : 0;
    bal_mosfet[2] = (( cell_3_v_over_max || cell_3_v_over_other ) && bal_temp[2] < max_b_t) ? 1 : 0;
  } else {
    bal_mosfet[0] = (cell_1_v_over_max  && bal_temp[0] < max_b_t) ? 1 : 0;
    bal_mosfet[1] = (cell_2_v_over_max  && bal_temp[1] < max_b_t) ? 1 : 0;
    bal_mosfet[2] = (cell_3_v_over_max  && bal_temp[2] < max_b_t) ? 1 : 0;
  }
  //Serial.println("Bal mosfets states are updated");
}

void set_bal_mosfets(bool bal_mosfet_0, bool bal_mosfet_1, bool bal_mosfet_2) {
  digitalWrite(bal_mosfet_cell_1_pin, bal_mosfet_0);
  digitalWrite(bal_mosfet_cell_2_pin, bal_mosfet_1);
  digitalWrite(bal_mosfet_cell_3_pin, bal_mosfet_2);
}