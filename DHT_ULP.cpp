/* DHT_ULP library

MIT license
written by Benjamim Krug
*/

#include "DHT_ULP.h"

RTC_DATA_ATTR ulp_var_t byte_count;
RTC_DATA_ATTR ulp_var_t dht_error_code;
RTC_DATA_ATTR ulp_var_t lowCycle;
RTC_DATA_ATTR ulp_var_t checkSum;
RTC_DATA_ATTR ulp_var_t dht_values[5];

DHT_ULP::DHT_ULP(gpio_num_t _dat_pin, uint8_t _dht_type) {
  dat_pin = _dat_pin;
  _type = _dht_type;
  _firstreading = true;
}

esp_err_t DHT_ULP::begin() {
  enum {
    FINISH_READING,
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
    I_GPIO_OUTPUT_EN(dat_pin),     //pinMode(dat_pin, OUTPUT);
    I_GPIO_SET(dat_pin, 0),        //digitalWrite(dat_pin, LOW);
    M_DELAY_US_5000_20000(18000),  //delay(18);

    I_GPIO_OUTPUT_DIS(dat_pin),
    I_GPIO_INPUT_EN(dat_pin),   //pinMode(dat_pin, INPUT);
    I_MOVI(R1, 0),              //R1 = 0;
    M_DELAY_US_10_100(10),      //delayMicros(10); wait a bit for the sensor to have time to wake up and respond

    //GET FIRST low and high pulses to indicate sensor presence
    I_MOVI(R2, 0),                     //R2 = 0; zero the loop count
    M_LABEL(EXPECT_FIRST_PULSE_LOW),   //expect low pulse
    I_ADDI(R2, R2, 1),                 //R2++;
    I_MOVR(R0, R2),                    //R0 = R2; moves R2 into R0 to compare it with MAX_CYCLES
    M_BGE(ERROR_TIMEOUT, MAX_CYCLES),  //if(R0 > MAX_CYCLES) goto ERROR_TIMEOUT;
    I_GPIO_READ(dat_pin),              //R0 = digitalRead(dat_pin);
    M_BL(EXPECT_FIRST_PULSE_LOW, 1),   //if (R0 < 1) goto EXPECT_FIRST_PULSE_LOW; effectively creates a do while() loop

    I_MOVI(R2, 0),                      //R2 = 0; zero the loop count
    M_LABEL(EXPECT_FIRST_PULSE_HIGH),   //expect high pulse
    I_ADDI(R2, R2, 1),                  //R2++;
    I_MOVR(R0, R2),                     //R0 = R2, moves R2 into R0 to compare it with MAX_CYCLES
    M_BGE(ERROR_TIMEOUT, MAX_CYCLES),   //if(R0 > MAX_CYCLES) goto ERROR_TIMEOUT;
    I_GPIO_READ(dat_pin),               //R0 = digitalRead(dat_pin);
    M_BGE(EXPECT_FIRST_PULSE_HIGH, 1),  //if (R0 >= 1) goto EXPECT_FIRST_PULSE_HIGH;

    //receive 5 bytes of data
    M_LABEL(RECEIVE_DATA),
    I_MOVI(R3, 0),//R3 = 0; zeros the current byte

    //receive 1 byte of data
    M_LABEL(RECEIVE_BYTE),

    //Expect a low pulse of 50us
    I_MOVI(R2, 0),                     //R2 = 0;
    M_LABEL(EXPECT_PULSE_LOW),         //expect low pulse
    I_ADDI(R2, R2, 1),                 //R2++;
    I_MOVR(R0, R2),                    //R0 = R2; moves R2 into R0 to compare it with MAX_CYCLES
    M_BGE(ERROR_TIMEOUT, MAX_CYCLES),  //if(R0 > MAX_CYCLES) goto ERROR_TIMEOUT;
    I_GPIO_READ(dat_pin),              //R0 = digitalRead(dat_pin);
    M_BL(EXPECT_PULSE_LOW, 1),         //if (R0 == 0) goto EXPECT_PULSE_LOW; 
    //it is checking if it is smaller than 1, but since the only value smaller than 1 in this case is 0 it has the same effect as R0 == 0
    I_MOVI(R0, 0),                     //R0 = 0; only needed because for I_PUT
    //we need to set R1 to 0 because every I_PUT call treats the RTC variable as an array essentially, as bellow:
    I_PUT(R2, R0, lowCycle),           //lowCycle[R0] = R2;

    //Expect a high pulse of 28us or 70us
    I_MOVI(R2, 0),                     //R2 = 0;
    M_LABEL(EXPECT_PULSE_HIGH),        //expect high pulse
    I_ADDI(R2, R2, 1),                 //R2++
    I_MOVR(R0, R2),                    //R0 = R2;, moves R2 into R0 to compare it with MAX_CYCLES
    M_BGE(ERROR_TIMEOUT, MAX_CYCLES),  //if(R0 > MAX_CYCLES) goto ERROR_TIMEOUT;
    I_GPIO_READ(dat_pin),              //R0 = digitalRead(dat_pin);
    M_BGE(EXPECT_PULSE_HIGH, 1),       //if(R0 >= 1) goto EXPECT_PULSE_HIGH; keep waiting to get the end of the high pulse

    //Process the Pulse information that it received
    M_LABEL(PROCESS_PULSE),  //PROCESS PULSE information
    I_LSHI(R3, R3, 1),       //R3 <<= 1; left shifts the current byte since the received data is MSB first
    I_GET(R0, R0, lowCycle), //R0 = lowCycle[R0]; load the low cycle value so we can compare it; 
    // we can use R0 as the index here because we know that if it got to this point the previous digitalRead resulted in 0 and that value is in R0
    I_SUBR(R0, R0, R2),            //R0 = R0 - R2, gets the diference between the low cycle and high cycle
    M_BL(FINISH_BIT_READ, 32767),  //if(highCycle - lowCycle < 32767), basically if(highCycle < lowCycle), then it skips the bit set

    I_ORI(R3, R3, 1),           //R3 |= 1; sets the current bit, won't be executed

    M_LABEL(FINISH_BIT_READ),  //finish the process of bit reading
    I_ADDI(R1, R1, 1),         //R1++;
    I_MOVR(R0, R1),            //R0 = R1; moves R1 into R0 to use in the comparison
    M_BL(RECEIVE_BYTE, 8),     //if (R0 < 8) goto RECEIVE_BYTE; read another bit of data if we haven't read 8 yet

    I_MOVI(R1, 0),              //R1 = 0, resets the bit counter
    I_GET(R0, R1, byte_count),  //R0 = byte_count; loads byte_count into R0 to work with it
    I_PUT(R3, R0, dht_values),  //dht_values[R0] = R3; saves the current byte into RTC memory
    I_ADDI(R0, R0, 1),          //R0++;
    I_PUT(R0, R1, byte_count),  //byte_count = R0; saves the byte_count back into RTC memory
    M_BL(RECEIVE_DATA, 5),      //if (R0 < 5) goto RECEIVE_BYTE; while it hasn't received 5 bytes of data

    //Checksum test to verify if the data has been received correctly
    I_MOVI(R2, 0),             //R2 = 0; resets the data position to read all 5 bytes again
    I_MOVI(R3, 0),             //R3 = 0; resets the register to use for the checksum

    M_LABEL(CHECKSUM_TEST),
    I_GET(R1, R2, dht_values),//R1 = dht_values[R0];
    I_ADDR(R3, R3, R1),       //R3 += R1;
    I_ADDI(R2, R2, 1),        //R2++;
    I_MOVR(R0, R2),           //R0 = R2, moves R2 into R0 to compare with MAX_CYCLES
    M_BL(CHECKSUM_TEST, 4),   //if (R0 < 4) goto CHECKSUM_TEST; keeps adding the values

    I_GET(R1, R2, dht_values),//R1 = dht_values[R2]; read dht_values[4] into R1, to be compared against the checksum
    I_ANDI(R3, R3, 0xFF),     //R3 |= 0xFF; logical AND of the checksum and 256
    I_MOVI(R2, 0),            //R2 = 0; just used for the I_PUT instruction
    I_PUT(R3, R2, checkSum),  //checkSum[R2] = R3;
    I_SUBR(R0, R3, R1),       //R0 = R3 - R1; we subtract one from another in order to compare them
    I_MOVI(R3, ESP_OK),       //R3 = ESP_OK; 
    //this is the only spot where we can set the error code as ESP_OK, if there is a checksum problem it'll be overwritten

    M_BL(FINISH_READING, 1),  //if(R0 == 0) goto FINISH_READING;
	  // just as before, by checking if it is smaller than 1, we are actually checking if it is equal to 0

    M_BX(ERROR_CHECKSUM),    // goto ERROR_CHECKSUM; 
    //if it gets to this point there has been a corruption in the data since the checkSum and dhtValues[4] differ, so jump to checksum error label

    M_LABEL(ERROR_TIMEOUT),
    I_MOVI(R3, ESP_ERR_TIMEOUT), // R3 = ESP_ERR_TIMEOUT;
    M_BX(FINISH_READING),        // goto FINISH_READING; jump to FINISH_READING unconditionally

    M_LABEL(ERROR_CHECKSUM),
    I_MOVI(R3, ESP_ERR_INVALID_CRC),// R3 = ESP_ERR_INVALID_CRC; invalid CRC code is used to indicate invalid Checksum
	
    M_LABEL(FINISH_READING),
    I_MOVI(R2, 0),                 //R2 = 0, just used for the I_PUT instruction
    I_PUT(R3, R2, dht_error_code), //dht_error_code[R0] = R3; saves the error code into RTC memory
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

  if (dht_error_code.val == ESP_ERR_INVALID_CRC) {
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
  ulp_run(0);
  hulp_peripherals_on();
  _lastresult = true;
  return _lastresult;
}