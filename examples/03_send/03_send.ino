/*
 * 1. once the device has joined the network
 * 2. send uplink messages using send()
 */

#include "loramodem.h"
#include "keys.h"

LoRaWANModem modem;
Status join_state;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  modem.begin();
  modem.info();
  join_state = modem.join(appeui, appkey);
}


void loop() {
  delay(10000);
  Serial.println("sending");
  uint8_t payload[11] = { 0x6d, 0x61, 0x6b, 0x65, 0x20, 0x7a, 0x75, 0x72, 0x69, 0x63, 0x68 };

  if (join_state == OK) {
    modem.send(payload, 11);
  }
}
