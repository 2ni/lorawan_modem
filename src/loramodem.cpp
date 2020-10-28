#include <Arduino.h>
#include "loramodem.h"

LORAMODEM::LORAMODEM() : modem(LORA_TX, LORA_RX, NC, NC) {
  _pin_cts = 2;
  _pin_rts = 3;
}

LORAMODEM::LORAMODEM(uint8_t pin_cts, uint8_t pin_rts) : modem(LORA_TX, LORA_RX, NC, NC) {
  _pin_cts = pin_cts;
  _pin_rts = _pin_rts;
}

void LORAMODEM::begin() {
  pinMode(_pin_rts, OUTPUT);
  digitalWrite(_pin_rts, HIGH);
  pinMode(_pin_cts, INPUT);

  modem.begin(115200); // data bits 8, stop bits 1, parity none
}

uint8_t LORAMODEM::_calc_crc(uint8_t cmd, const uint8_t *payload, uint8_t len) {
  uint8_t crc = cmd^len;
  if (payload == NULL) return crc;

  for (uint8_t i=0; i<len; i++) {
    crc ^= payload[i];
  }

  return crc;
}

Status LORAMODEM::command(Lora_cmd cmd, uint8_t *payload, uint8_t len_payload, uint8_t *response, uint8_t *len_response) {
  Status sw = write(cmd, payload, len_payload);
  if (sw != OK) return sw;

  return read(response, len_response);
}

Status LORAMODEM::command(Lora_cmd cmd, uint8_t *response, uint8_t *len_response) {
  return command(cmd, NULL, 0, response, len_response);
}

Status LORAMODEM::write(Lora_cmd cmd) {
  return write(cmd, NULL, 0);
}

Status LORAMODEM::write(Lora_cmd cmd, uint8_t *payload, uint8_t len) {
  digitalWrite(_pin_rts, LOW);
  unsigned long now = millis();
  // wait for modem to set busy line low with 10ms timeout
  while (digitalRead(_pin_cts) == HIGH) {
    if ((millis()-now) > 10) {
      Serial.println(DBG_ERR("cts timeout"));
      digitalWrite(_pin_rts, HIGH);
      return TIMEOUT;
    }
  }

  modem.write(cmd);
  modem.write(len);

  for (uint8_t i=0; i<len; i++) {
    modem.write(payload[i]);
  }
  modem.write(_calc_crc(cmd, payload, len));

  delay(25);

  digitalWrite(_pin_rts, HIGH);

  // wait for modem to set busy line high again
  now = millis();
  while (digitalRead(_pin_cts) == LOW) {
    if ((millis()-now) > 200) {
      Serial.println(DBG_ERR("cts release timeout"));
      return TIMEOUT;
    }
  }
  return OK;
}

Status LORAMODEM::read(uint8_t *payload, uint8_t *len) {
  unsigned long now = millis();
  // wait for data with 100ms timeout
  while (!modem.available()) {
    if ((millis()-now) > 100) {
      Serial.println(DBG_ERR("receive timeout"));
      return TIMEOUT;
    }
  }

  Status rc = (Status)modem.read();
  if (rc != OK) {
    Serial.printf(DBG_ERR("receive error: 0x%02x") "\n", rc);
    return rc;
  }

  while (!modem.available()) {}
  uint8_t l = modem.read();
  if (l) {
    modem.readBytes(payload, l);
  }

  while (!modem.available()) {}
  uint8_t chk = modem.read();

  if (chk != _calc_crc(rc, payload, l)) {
    Serial.printf(DBG_ERR("invalid crc: 0x%02x") "\n", chk);
    return BADCRC;
  }

  *len = l;
  return OK;
}

void LORAMODEM::print_arr(const char *name, uint8_t *arr, uint8_t len) {
  Serial.printf("%s:", name);
  for (uint8_t i=0; i<len; i++) {
    Serial.printf(" %02x", arr[i]);
  }
  Serial.println("");
}
