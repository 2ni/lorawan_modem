/*
 * 1. use app EUI and app key from TTN and set it in the module
 * 2. join the network using OTAA
 */

#include "loramodem.h"

LoRaWANModem modem;

const uint8_t appeui[8]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
const uint8_t appkey[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

void setup() {
  Serial.begin(9600);
  while (!Serial);

  modem.begin();
  modem.info();
  modem.join(appeui, appkey);

  unsigned long current_time = millis();
  while (modem.is_joining()) {
    if ((millis()-current_time) > 1000) {
      current_time = millis();
      Serial.println("waiting ...");
    }
  }
  Serial.println("joined");

}

void loop() {
}