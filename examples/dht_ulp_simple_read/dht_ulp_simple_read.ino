
/*
        Example of how to use the ESP32 ULP to read the DHT family of sensors.
        The whole read process is contained in the ULP code, which is all inside
   of the library. When you want to perform a reading you must firstly call
   .startReading() to initiate the ULP, then wait 300ms(so the ULP has time to
   comunicate everything) and get the results from the RTC memory The whole ULP
   process is abstracted by the library, no need for the user to know whats
   going on in the background.
*/
#include "DHT_ULP.h"

#define DHT_PIN GPIO_NUM_4  // GPIO_NUM, needs to be like this
#define DHT_TYPE DHT11
// #define DHT_TYPE DHT22
// #define DHT_TYPE DHT21

DHT_ULP dht(DHT_PIN, DHT_TYPE);

void setup() {
  Serial.begin(115200);
  printf("DHT ULP simple reading example!\n");
  dht.begin();
}

void loop() {
  dht.startReading();
  vTaskDelay(300);//needs to wait 300ms for the sensor reading to be done
  if (dht.getStatus() == ESP_OK) { //checks if there wasn't any CRC errors
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    // Check if any reads failed and prints an error message
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {  // only makes this process if all the read values are correct
      // Compute heat index in Fahrenheit (the default)
      float hif = dht.computeHeatIndex(f, h);
      // Compute heat index in Celsius (isFahreheit = false)
      float hic = dht.computeHeatIndex(t, h, false);
      // prints the values
      Serial.printf(
          "Humidity: %.2f %%\tTemperature: %.2f*C  %.2f*F \tHeat index: %.2f*C "
          "%.2f*F\n",
          h, t, f, hic, hif);
    }
  } else
    Serial.printf("DHT reading error: %i\n", dht.getStatus());
  vTaskDelay(850);  
  // adding up both delays, ends up with 1150ms instead of the 1000ms that is specified by the DHT, however, 
  // it does need to be this time, even in other libraries it is necessary
}