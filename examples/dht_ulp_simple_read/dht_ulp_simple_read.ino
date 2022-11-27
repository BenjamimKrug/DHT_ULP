
/* 
	Example of how to use the ESP32 ULP to read the DHT family of sensors.
	The whole read process is contained in the ULP code, which is all inside of the library.
	When you want to perform a reading you must firstly call .startReading() to initiate the ULP,
	then wait 300ms(so the ULP has time to comunicate everything) and get the results from the RTC memory
	The whole ULP process is abstracted by the library, no need for the user to know whats going on in the background.
*/
#include "DHT_ULP.h"

#define DHT_PIN GPIO_NUM_2 //GPIO_PIN, needs to be like this
#define DHT_TYPE DHT11
//#define DHT_TYPE DHT22
//#define DHT_TYPE DHT21

DHT_ULP dht(DHT_PIN, DHT_TYPE);

void setup() {
  Serial.begin(115200);
  printf("DHT ULP simple reading example!\n");
  dht.begin();
}

void loop() {
  dht.startReading();
  delay(300);
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");
  delay(700);
}