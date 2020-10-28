Arduino library to communicate with the murata lorawan board.

### Wiring
- you need to connect power with the power on top or it won't work
- connect it then as follows to your arduino nano ble:

| Type      | Murata | Nano     | Type      |
| --------- | ------ | -------- | --------- |
|           | GND    | GND      |           |
| COMMAND   | PB8    | D3       | RTS       |
| BUSY      | PA8    | D2       | CTS       |
| USART1_RX | PA10   | TX       | UART TX   |
| USART1_TX | PA9    | RX       | UART RX   |

Or you can define your own pins for RTS, CTS and define them as follows:
```
LORAMODEM modem(uint8_t pin_cts, uint8_t pin_rts);
```

![wiring](images/wiring.jpg "example of wiring")

### Usage
```cpp
/*
 * 1. run modem.info() to get dev eui
 * 2. create application and device on ttn with dev eui
 * 3. use app eui and app key from ttn and set it in murata with CMD_SETJOINEUI, CMD_SETNWKKEY
 *
 */

#include "loramodem.h"

LORAMODEM modem;

void setup() {
  modem.begin();
  /*
  const uint8_t appeui[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  modem.cmd_and_result("set app eui", CMD_SETJOINEUI, appeui, 8);
  const uint8_t appkey[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  modem.cmd_and_result("set app key", CMD_SETNWKKEY, appkey, 16);
  */
  modem.info();
  modem.cmd_and_result("join", CMD_JOIN);

  //                     port, unconf, data
  uint8_t payload[5] = { 0x01, 0x00, 0x12, 0x13, 0x14 };
  modem.cmd_and_result("send data", CMD_REQUESTTX, payload, 5);
}

void loop() {
}
```
