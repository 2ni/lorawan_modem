/*
 * Retrieve device information and use Dev EUI to register it on the network
 */

#include "loramodem.h"

LoRaWANModem modem;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  modem.begin();
  modem.info();
}

void loop() {
}