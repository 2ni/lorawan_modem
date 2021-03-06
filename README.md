Arduino library to communicate with the lorawan module Murata [ST B-L072Z-LRWAN1](https://www.st.com/en/evaluation-tools/b-l072z-lrwan1.html) or the [Miromico FMLR-72-U-STL0Z](https://miromico.ch/portfolio/fmlr_stm/).

### Wiring

- you need to connect power with the power on top or it won't work
- connect it then as follows to your Arduino Nano 33 BLE:

| Type      | Murata | Nano     | Type      |
| --------- | ------ | -------- | --------- |
|           | GND    | GND      |           |
| COMMAND   | PB8    | D3       | RTS       |
| BUSY      | PA8    | D2       | CTS       |
| USART1_RX | PA10   | TX       | UART TX   |
| USART1_TX | PA9    | RX       | UART RX   |


| Type      | Miromico | Nano     | Type      |
| --------- | ------   | -------- | --------- |
|           | GND      | GND      |           |
| COMMAND   | PA8      | D3       | RTS       |
| BUSY      | PB1      | D2       | CTS       |
| UART2_RX  | PA3      | TX       | UART TX   |
| UART2_TX  | PA2      | RX       | UART RX   |

Or you can define your own pins for RTS, CTS and define them as follows:

```
LoRaWANModem modem(uint8_t pin_cts, uint8_t pin_rts);
```

![wiring](images/wiring_fritzing.jpg "example of wiring")

### Usage

#### Register your device on the network

```cpp
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
```

#### Join the network using OTAA

```cpp
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
}
```

#### Send packets

```cpp
/*
 * 1. once the device has joined the network
 * 2. send uplink messages using send()
 */

#include "loramodem.h"

LoRaWANModem modem;

const uint8_t appeui[8]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
const uint8_t appkey[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

Status join_state;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  modem.begin();
  modem.info();
  join_status = modem.join(appeui, appkey);
}


void loop() {
  delay(10000);
  Serial.println("sending");
  uint8_t payload[11] = { 0x6d, 0x61, 0x6b, 0x65, 0x20, 0x7a, 0x75, 0x72, 0x69, 0x63, 0x68 };
  if (join_state == OK) {
    modem.send(payload, 11);
  }
}
```
