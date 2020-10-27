Arduino library to communicate with the murata lorawan board.

### Usage
```
#include "loramodem.h"

LORAMODEM modem;

void setup() {
  modem.begin();
  uint8_t response[255] = {0};
  uint   len = 0;
 
  modem.command(CMD_GETVERSION, response, &len);
  modem.print_arr("response", response, len);
}

void loop() {
}
```
