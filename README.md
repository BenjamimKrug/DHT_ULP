This is an Arduino ESP32 library to read the DHT series of low cost temperature/humidity sensors using the ULP of the ESP32 SoC.
All the ESP32 ULP processes are abstracted by the library and are written using the C MACROS given by ESP-IDF to give maximum compatiblity.
A simple example of how to use the library can be found in dht_ulp_simple_read.ino

# Supported GPIOs for the ESP32
GPIO0, GPIO2, GPIO4, GPIO12 ~ GPIO15, GPIO25 ~ GPIO33

Special thanks to boarchuz(https://github.com/boarchuz), since part of the code works on top of a simplified version of his HULP library(https://github.com/boarchuz/HULP).
Obs.: At the current state, no other ULP process can be used along with this library.
