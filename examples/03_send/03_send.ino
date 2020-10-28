/*
 * 1. once the device has joined the network
 * 2. send uplink messages using send()
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
  delay(10000);
  Serial.println("sending");
  uint8_t payload[11] = { 0x6d, 0x61, 0x6b, 0x65, 0x20, 0x7a, 0x75, 0x72, 0x69, 0x63, 0x68 };
  modem.send(payload, 11);
}