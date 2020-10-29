/*
 * 1. use app EUI and app key from TTN and set it in the module
 * 2. join the network using OTAA
 */

#include "loramodem.h"
#include "keys.h"

LoRaWANModem modem;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  modem.begin();
  modem.info();
  modem.join(appeui, appkey);

  unsigned long current_time = millis();
  Serial.print("waiting ");
  while (modem.is_joining()) {
    if ((millis()-current_time) > 1000) {
      current_time = millis();
      Serial.print(".");
    }
  }
  Serial.println("joined");

}

void loop() {
}
