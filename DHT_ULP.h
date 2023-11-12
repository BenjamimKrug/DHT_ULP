/* DHT_ULP library

MIT license
written by Benjamim Krug
*/
#ifndef DHT_ULP_H
#define DHT_ULP_H

#define CONFIG_HULP_MACRO_OPTIMISATIONS
// #define CONFIG_HULP_USE_APPROX_FAST_CLK
#include "esp_err.h"
#include "hulp.h"
#include <Arduino.h>

#define MAX_CYCLES 500

enum {
  DHT11,
  DHT22,
  DHT21,
};


class DHT_ULP {
public:
  DHT_ULP(gpio_num_t _dat_pin, uint8_t _dht_type);
  esp_err_t begin();
  void print_values();
  bool startReading();
  float readTemperature(bool S = false);
  float convertCtoF(float);
  float convertFtoC(float);
  float computeHeatIndex(float temperature, float percentHumidity, bool isFahrenheit = true);
  float readHumidity(void);

private:
  gpio_num_t dat_pin;
  uint8_t _type = DHT22;
  uint32_t _lastreadtime;
  bool _firstreading;
  bool _lastresult;
};
#endif