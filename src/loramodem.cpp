#include <Arduino.h>
#include "loramodem.h"

LoRaWANModem::LoRaWANModem(uint8_t pin_cts, uint8_t pin_rts) : uart(LORA_TX, LORA_RX, NC, NC) {
  _pin_cts = pin_cts;
  _pin_rts = pin_rts;
}

void LoRaWANModem::begin() {
  pinMode(_pin_rts, OUTPUT);
  digitalWrite(_pin_rts, HIGH);
  pinMode(_pin_cts, INPUT);

  uart.begin(115200); // data bits 8, stop bits 1, parity none
}

Status LoRaWANModem::join(const uint8_t *appeui, const uint8_t *appkey) {
  Status s;

  s = write(CMD_SETJOINEUI, appeui, 8);
  if (s != OK) {
    Serial.printf(DBG_ERR("set app eui error: 0x%02x") "\n", (uint8_t)s);
    return s;
  }

  s = write(CMD_SETNWKKEY, appkey, 16);
  if (s != OK) {
    Serial.printf(DBG_ERR("set appkey error: 0x%02x") "\n", (uint8_t)s);
    return s;
  }

  s = write(CMD_JOIN);
  if (s != OK) {
    Serial.printf(DBG_ERR("join error: 0x%02x") "\n", (uint8_t)s);
    return s;
  }
  return OK;
}

bool LoRaWANModem::is_joining(void (*join_done)(Event_code code)) {
  uint8_t len;
  uint8_t response[3] = {0};
  Status s = command(CMD_GETEVENT, response, &len);
  if (s != OK) {
    Serial.printf(DBG_ERR("join event cmd error: 0x%02x") "\n", (uint8_t)s);
  }
  Serial.printf("join resp: 0x%02x\n", response[0]);

  if (response[0] == EVT_JOINED || response[0] == EVT_JOINFAIL) {
    if (join_done != NULL) {
      join_done((Event_code)response[0]);
    }
    return false;
  }

  return true;
}

bool LoRaWANModem::is_joining() {
  return is_joining(NULL);
}

Status LoRaWANModem::send(const uint8_t *data, uint8_t len, uint8_t port, uint8_t confirm) {
  uint8_t payload_len = len + 2;
  uint8_t payload[255];

  payload[0] = port;
  payload[1] = confirm;
  for (uint8_t i=0; i<len; i++) {
    payload[2+i] = data[i];
  }

  Status sw = write(CMD_REQUESTTX, payload, payload_len);
  if (sw != OK) {
    Serial.printf(DBG_ERR("tx cmd error: 0x%02x") "\n", (uint8_t)sw);
    return sw;
  }

  /*
  uint8_t event_len;
  uint8_t event_resp[3] = {0};

  Status se = command(CMD_GETEVENT, event_resp, &event_len);
  if (se != OK) {
    Serial.printf(DBG_ERR("tx event cmd error: %02x") "\n", (uint8_t)se);
  }

  if (event_resp[0] == EVT_TXDONE) {
    tx_done((Event_code)event_resp[0]);
    return false;
  }
  */

  return OK;
}

uint8_t LoRaWANModem::_calc_crc(uint8_t cmd, const uint8_t *payload, uint8_t len) {
  uint8_t crc = cmd^len;
  if (payload == NULL) return crc;

  for (uint8_t i=0; i<len; i++) {
    crc ^= payload[i];
  }

  return crc;
}

Status LoRaWANModem::command(Lora_cmd cmd, const uint8_t *payload, uint8_t len_payload, uint8_t *response, uint8_t *len_response) {
  Status sw = write(cmd, payload, len_payload);
  if (sw != OK) return sw;

  return read(response, len_response);
}

Status LoRaWANModem::command(Lora_cmd cmd, uint8_t *response, uint8_t *len_response) {
  return command(cmd, NULL, 0, response, len_response);
}

Status LoRaWANModem::write(Lora_cmd cmd) {
  return write(cmd, NULL, 0);
}

Status LoRaWANModem::write(Lora_cmd cmd, const uint8_t *payload, uint8_t len) {
  digitalWrite(_pin_rts, LOW);
  unsigned long now = millis();
  // wait for uart to set busy line low with 10ms timeout
  while (digitalRead(_pin_cts) == HIGH) {
    if ((millis()-now) > 10) {
      Serial.println(DBG_ERR("cts timeout"));
      digitalWrite(_pin_rts, HIGH);
      return TIMEOUT;
    }
  }

  uart.write(cmd);
  uart.write(len);

  for (uint8_t i=0; i<len; i++) {
    uart.write(payload[i]);
  }
  uart.write(_calc_crc(cmd, payload, len));

  delay(25);

  digitalWrite(_pin_rts, HIGH);

  // wait for uart to set busy line high again
  now = millis();
  while (digitalRead(_pin_cts) == LOW) {
    if ((millis()-now) > 200) {
      Serial.println(DBG_ERR("cts release timeout"));
      return TIMEOUT;
    }
  }
  delay(25); // seems to many write at a time results to cts timeouts
  return OK;
}

Status LoRaWANModem::read(uint8_t *payload, uint8_t *len) {
  unsigned long now = millis();
  // wait for data with 100ms timeout
  while (!uart.available()) {
    if ((millis()-now) > 100) {
      Serial.println(DBG_ERR("receive timeout"));
      return TIMEOUT;
    }
  }

  Status rc = (Status)uart.read();
  if (rc != OK) {
    Serial.printf(DBG_ERR("receive error: 0x%02x") "\n", rc);
    return rc;
  }

  while (!uart.available()) {}
  uint8_t l = uart.read();
  if (l) {
    uart.readBytes(payload, l);
  }

  while (!uart.available()) {}
  uint8_t chk = uart.read();

  if (chk != _calc_crc(rc, payload, l)) {
    Serial.printf(DBG_ERR("invalid crc: 0x%02x") "\n", chk);
    return BADCRC;
  }

  *len = l;
  return OK;
}

void LoRaWANModem::info() {
  cmd_and_result("version", CMD_GETVERSION);
  cmd_and_result("status", CMD_GETSTATUS);
  cmd_and_result("chip id", CMD_GETCHIPEUI);
  cmd_and_result("dev eui", CMD_GETDEVEUI);
  cmd_and_result("app eui", CMD_GETJOINEUI);
  cmd_and_result("region", CMD_GETREGION);
}

void LoRaWANModem::cmd_and_result(const char *name, Lora_cmd cmd) {
  cmd_and_result(name, cmd, NULL, 0);
}

void LoRaWANModem::cmd_and_result(const char *name, Lora_cmd cmd, const uint8_t *payload, uint8_t len_payload) {
  uint8_t response[255] = {0};
  uint8_t len = 0;

  Serial.printf("%s: ", name);

  if (command(cmd, payload, len_payload, response, &len) == OK) {
    Serial.print(DBG_OK("ok"));
    print_arr(response, len);
  } else {
    Serial.println(DBG_ERR("failed"));
  }
}

void LoRaWANModem::print_arr(uint8_t *arr, uint8_t len) {
  for (uint8_t i=0; i<len; i++) {
    Serial.printf(" %02x", arr[i]);
  }
  Serial.println("");
}
