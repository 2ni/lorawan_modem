/*
 * 1. once the device has joined the network
 * 2. send uplink messages using send()
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
  Serial.println("waiting ");
  while (modem.is_joining()) {
    if ((millis()-current_time) > 1000) {
      current_time = millis();
      Serial.print(".");
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
