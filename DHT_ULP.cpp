/* DHT_ULP library

MIT license
written by Benjamim Krug
*/

#include "DHT_ULP.h"

RTC_DATA_ATTR ulp_var_t byte_count;
RTC_DATA_ATTR ulp_var_t dht_error_code;
RTC_DATA_ATTR ulp_var_t lowCycle;
RTC_DATA_ATTR ulp_var_t checkSum;
RTC_DATA_ATTR ulp_var_t diference[8];
RTC_DATA_ATTR ulp_var_t dht_values[5];

DHT_ULP::DHT_ULP(gpio_num_t _dat_pin, uint8_t _dht_type) {
  dat_pin = _dat_pin;
  _type = _dht_type;
  _firstreading = true;
}

esp_err_t DHT_ULP::begin() {
  enum {
    FINISH_READING,
    REQUEST_DATA,
    RECEIVE_DATA,
    RECEIVE_BYTE,
    EXPECT_FIRST_PULSE_LOW,
    EXPECT_FIRST_PULSE_HIGH,
    EXPECT_PULSE_LOW,
    EXPECT_PULSE_HIGH,
    FINISH_BIT_READ,
    PROCESS_PULSE,
    ERROR_TIMEOUT,
    ERROR_CHECKSUM,
    CHECKSUM_TEST,
    DELAY_250,
  };

  const ulp_insn_t program[] = {
    I_GPIO_OUTPUT_DIS(dat_pin),
    I_GPIO_INPUT_EN(dat_pin),  //pinMode(DHT_PIN, INPUT)
    I_GPIO_PULLUP(dat_pin, 1),
    I_GPIO_SET(dat_pin, 1),  //digitalWrite(DHT_PIN, HIGH)
    M_LABEL(DELAY_250),
    M_DELAY_US_5000_20000(10000),
    I_ADDI(R0, R2, 1),  //R2++
    I_MOVR(R2, R0),     //R0 = R2, moves R2 into R0 to compare with MAX_CYCLES
    M_BL(DELAY_250, 25),

    I_GPIO_OUTPUT_EN(dat_pin),     //pinMode(DHT_PIN, OUTPUT)
    I_GPIO_SET(dat_pin, 0),        //digitalWrite(DHT_PIN, LOW)
    M_DELAY_US_5000_20000(20000),  //delay(20)

    M_LABEL(REQUEST_DATA),   //start requesting data
    I_GPIO_SET(dat_pin, 1),  //digitalWrite(DHT_PIN, HIGH)
    M_DELAY_US_10_100(40),   //delayMicros(40)


    I_GPIO_OUTPUT_DIS(dat_pin),
    I_GPIO_INPUT_EN(dat_pin),   //pinMode(DHT_PIN, INPUT)
    I_MOVI(R2, 0),              //R2 = 0
    I_MOVI(R1, 0),              //R1 = 0, resets the bit counter
    I_PUT(R2, R1, byte_count),  //saves 0 into byte_count variable on the RTC memory
    M_DELAY_US_10_100(10),      //delayMicros(40)

    //GET FIRST 80us low and high pulses
    I_MOVI(R2, 0),                     //R2 = 0, count = 0
    M_LABEL(EXPECT_FIRST_PULSE_LOW),   //expect low pulse
    I_ADDI(R0, R2, 1),                 //R2++
    I_MOVR(R2, R0),                    //R0 = R2, moves R2 into R0 to compare with MAX_CYCLES
    M_BGE(ERROR_TIMEOUT, MAX_CYCLES),  //if(R0 > max_cycles), if the count has passed the maximum value
    I_GPIO_READ(dat_pin),              //R0 = digitalRead(dht_pin)
    M_BL(EXPECT_FIRST_PULSE_LOW, 1),   //while(R0 < 1)

    I_MOVI(R2, 0),                      //R2 = 0, count = 0
    M_LABEL(EXPECT_FIRST_PULSE_HIGH),   //expect high pulse
    I_ADDI(R0, R2, 1),                  //R2++
    I_MOVR(R2, R0),                     //R0 = R2, moves R2 into R0 to compare with MAX_CYCLES
    M_BGE(ERROR_TIMEOUT, MAX_CYCLES),   //if(R0 > max_cycles), if the count has passed the maximum value
    I_GPIO_READ(dat_pin),               //R0 = digitalRead(dht_pin)
    M_BGE(EXPECT_FIRST_PULSE_HIGH, 1),  //while(R0 >= 1),


    I_MOVI(R1, 0),              //R1 = 0, resets the bit counter
    I_GET(R0, R1, byte_count),  //loads byte_count into the registers to work with it
    //receive 5 bytes of data
    M_LABEL(RECEIVE_DATA),
    I_GET(R3, R0, dht_values),//read the current byte of data

    //receive 1 byte of data
    M_LABEL(RECEIVE_BYTE),
    //Expect a low pulse of 50us
    I_MOVI(R2, 0),                     //R2 = 0, count = 0
    M_LABEL(EXPECT_PULSE_LOW),         //expect low pulse
    I_ADDI(R0, R2, 1),                 //R2++
    I_MOVR(R2, R0),                    //R0 = R2, moves R2 into R0 to compare with MAX_CYCLES
    M_BGE(ERROR_TIMEOUT, MAX_CYCLES),  //if(R0 > max_cycles), if the count has passed the maximum value
    I_GPIO_READ(dat_pin),              //R0 = digitalRead(dht_pin)
    M_BL(EXPECT_PULSE_LOW, 1),         //while(R0 < 1)
    I_MOVI(R0, 0),
    I_PUT(R2, R0, lowCycle),

    //Expect a high pulse of 28us or 70us
    I_MOVI(R2, 0),                     //R2 = 0, count = 0
    M_LABEL(EXPECT_PULSE_HIGH),        //expect high pulse
    I_ADDI(R0, R2, 1),                 //R2++
    I_MOVR(R2, R0),                    //R0 = R2, moves R2 into R0 to compare with MAX_CYCLES
    M_BGE(ERROR_TIMEOUT, MAX_CYCLES),  //if(R0 > max_cycles), if the count has passed the maximum value
    I_GPIO_READ(dat_pin),              //R0 = digitalRead(dht_pin)
    M_BGE(EXPECT_PULSE_HIGH, 1),       //while(R0 >= 1),

    //Process the Pulse information that it received
    M_LABEL(PROCESS_PULSE),  //PROCESS PULSE information
    I_LSHI(R3, R3, 1),       //R1 <<= 1, left shifts the value since the received data is MSB first
    I_GET(R0, R0, lowCycle),
    I_SUBR(R0, R0, R2),             //R0 = R0 - R2, gets the diference between the low cycle and high cycle
    M_BL(FINISH_BIT_READ, 32767),  //if(highCycle - lowCycle < 0), basically if(highCycle < lowCycle), then it skips the bit set

    I_ORI(R3, R3, 1),           //R1 |= 1, sets the current bit

    M_LABEL(FINISH_BIT_READ),  //finish the process of bit reading
    I_ADDI(R1, R1, 1),         //R1++
    I_MOVR(R0, R1),            //moves R1 into R0 to use in the comparison
    M_BL(RECEIVE_BYTE, 8),     //while R0 < 8, read another bit of data

    I_MOVI(R1, 0),              //R1 = 0, resets the bit counter
    I_GET(R0, R1, byte_count),  //loads byte_count into R0 to work with it
    I_PUT(R3, R0, dht_values),
    I_ADDI(R0, R0, 1),          //byte_count++
    I_PUT(R0, R1, byte_count),  //saves byte_count back into RTC memory
    M_BL(RECEIVE_DATA, 5),      //while it hasn't received 5 bytes of data, goto RECEIVE_BYTE

    //Checksum test to verify if the data has been received correctly
    I_MOVI(R2, 0),             //R2 = 0, resets the data position to read all 5 bytes again
    I_MOVI(R3, 0),             //R3 = 0, resets the register to use for the checksum
    M_LABEL(CHECKSUM_TEST),
    I_GET(R1, R2, dht_values),//read the value from dht_values[R2] into R1
    I_ADDR(R3, R3, R1),       //R3 += R1
    I_ADDI(R0, R2, 1),        //R2++
    I_MOVR(R2, R0),           //R0 = R2, moves R2 into R0 to compare with MAX_CYCLES
    M_BL(CHECKSUM_TEST, 4),   //while R2 < 4, keeps adding the values
    I_GET(R1, R2, dht_values),//read dht_values[4] into R1, to be compared against the checksum
    I_ANDI(R3, R3, 0xFF),     //logical AND of the checksum and 256
    I_SUBR(R0, R3, R1),       //R3(checksum) minus R1(dht_values[4]) is put into R0
    I_MOVI(R3, ESP_OK),       //set R3 as ESP_OK
    M_BL(FINISH_READING, 1), //check if R0 is equal to 0, 
	//if the checksum is bigger than dht_values[4] R0 will obviously be bigger than 0, but if it is smaller than dht_values[4] R0 will be bigger than 32767 due to overflow
	//so checking if R0 is smaller than 1 solves for both situations
    M_BX(ERROR_CHECKSUM),    //there has been a corruption in the data, so jump to checksum error label

    M_LABEL(ERROR_TIMEOUT),
    I_MOVI(R3, ESP_ERR_TIMEOUT),
    M_BX(FINISH_READING),

    M_LABEL(ERROR_CHECKSUM),
    I_MOVI(R3, ESP_ERR_INVALID_CRC),//invalid CRC code is used to indicate invalid Checksum
	
    M_LABEL(FINISH_READING),
    I_MOVI(R2, 0),                 //R2 = 0, just used for the I_PUT instruction
    I_PUT(R3, R2, dht_error_code), //saves the error code into RTC memory
    I_END(),
    I_HALT(),
  };

  esp_err_t error = ESP_OK;

  error = hulp_configure_pin(dat_pin, RTC_GPIO_MODE_INPUT_OUTPUT, GPIO_PULLUP_ONLY, 0);//configures the GPIO pin as PULLUP

  size_t load_addr = 0;
  size_t size = sizeof(program) / sizeof(ulp_insn_t);
  error = ulp_process_macros_and_load(load_addr, program, &size);
  //Prepare ULP, but do not enable timer so it won't run yet (see ulp_run())
  REG_SET_FIELD(SENS_SAR_START_FORCE_REG, SENS_PC_INIT, 0);
  CLEAR_PERI_REG_MASK(SENS_SAR_START_FORCE_REG, SENS_ULP_CP_FORCE_START_TOP);
  SET_PERI_REG_MASK(RTC_CNTL_OPTIONS0_REG, RTC_CNTL_BIAS_I2C_FOLW_8M);
  SET_PERI_REG_MASK(RTC_CNTL_OPTIONS0_REG, RTC_CNTL_BIAS_CORE_FOLW_8M);
  SET_PERI_REG_MASK(RTC_CNTL_OPTIONS0_REG, RTC_CNTL_BIAS_SLEEP_FOLW_8M);
  return error;
}

float DHT_ULP::readTemperature(bool S) {
  float f = NAN;
  if (dht_error_code.val == ESP_OK) {
    switch (_type) {
      case DHT11:
        f = dht_values[2].val;
        if (S) {
          f = convertCtoF(f);
        }
        break;
      case DHT22:
      case DHT21:
        f = dht_values[2].val & 0x7F;
        f *= 256;
        f += dht_values[3].val;
        f /= 10;
        if (dht_values[2].val & 0x80)
          f *= -1;

        if (S)
          f = convertCtoF(f);
        break;
    }
  }
  return f;
}

float DHT_ULP::convertCtoF(float c) {
  return c * 9 / 5 + 32;
}

float DHT_ULP::convertFtoC(float f) {
  return (f - 32) * 5 / 9;
}

float DHT_ULP::readHumidity(void) {
  float f = NAN;
  if (dht_error_code.val == ESP_OK) {
    switch (_type) {
      case DHT11:
        f = dht_values[0].val;
        break;
      case DHT22:
      case DHT21:
        f = dht_values[0].val;
        f *= 256;
        f += dht_values[1].val;
        f /= 10;
        break;
    }
  }
  return f;
}


//boolean isFahrenheit: True == Fahrenheit; False == Celcius
float DHT_ULP::computeHeatIndex(float temperature, float percentHumidity, bool isFahrenheit) {
  // Adapted from equation at: https://github.com/adafruit/DHT-sensor-library/issues/9 and
  // Wikipedia: http://en.wikipedia.org/wiki/Heat_index
  if (!isFahrenheit) {
    // Celsius heat index calculation.
    return -8.784695 + 1.61139411 * temperature + 2.338549 * percentHumidity + -0.14611605 * temperature * percentHumidity + -0.01230809 * pow(temperature, 2) + -0.01642482 * pow(percentHumidity, 2) + 0.00221173 * pow(temperature, 2) * percentHumidity + 0.00072546 * temperature * pow(percentHumidity, 2) + -0.00000358 * pow(temperature, 2) * pow(percentHumidity, 2);
  } else {
    // Fahrenheit heat index calculation.
    return -42.379 + 2.04901523 * temperature + 10.14333127 * percentHumidity + -0.22475541 * temperature * percentHumidity + -0.00683783 * pow(temperature, 2) + -0.05481717 * pow(percentHumidity, 2) + 0.00122874 * pow(temperature, 2) * percentHumidity + 0.00085282 * temperature * pow(percentHumidity, 2) + -0.00000199 * pow(temperature, 2) * pow(percentHumidity, 2);
  }
}

void DHT_ULP::print_values() {
  printf("Woken up!%i\n", byte_count.val);
  printf("error code: %s\n", esp_err_to_name(dht_error_code.val));

  if (dht_values[4].val != ((dht_values[0].val + dht_values[1].val + dht_values[2].val + dht_values[3].val) & 0xFF) || dht_error_code.val == ESP_ERR_INVALID_CRC) {
    Serial.println(F("Checksum failure!"));
    Serial.println(checkSum.val);
  }
  Serial.print("\ndata:\n");
  for (uint8_t i = 0; i < 5; i++) {
    Serial.print(dht_values[i].val);
    Serial.print(",");
  }
  Serial.println();
}

bool DHT_ULP::startReading() {
  uint32_t currenttime = millis();
  if (currenttime < _lastreadtime) {
    // ie there was a rollover
    _lastreadtime = 0;
  }
  if (!_firstreading && ((currenttime - _lastreadtime) < 2000)) {
    return _lastresult;  // return last correct measurement
  }
  _firstreading = false;
  _lastreadtime = millis();
  byte_count.val = 0;
  dht_values[4].val = dht_values[3].val = dht_values[2].val = dht_values[1].val = dht_values[0].val = 0;
  esp_err_t error = ulp_run(0);
  hulp_peripherals_on();
  _lastresult = true;
  return _lastresult;
}