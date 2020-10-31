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
  while(!uart);
}

/*
 * only sends a join command
 * does not wait for network
 */
Status LoRaWANModem::command_join(const uint8_t *appeui, const uint8_t *appkey) {
  Status s;

  s = command(CMD_SETJOINEUI, appeui, 8);
  if (s != OK) {
    Serial.printf(DBG_ERR("set app eui error: 0x%02x") "\n", (uint8_t)s);
    return s;
  }

  s = command(CMD_SETNWKKEY, appkey, 16);
  if (s != OK) {
    Serial.printf(DBG_ERR("set appkey error: 0x%02x") "\n", (uint8_t)s);
    return s;
  }

  s = command(CMD_JOIN);
  if (s != OK) {
    Serial.printf(DBG_ERR("join error: 0x%02x") "\n", (uint8_t)s);
    return s;
  }
  delay(25);
  return OK;
}

bool LoRaWANModem::is_joining(void (*join_done)(Event_code code)) {
  uint8_t len;
  uint8_t response[3] = {0};
  Status s = command(CMD_GETEVENT, response, &len);
  if (s != OK) {
    Serial.printf(DBG_ERR("pulling join event error: 0x%02x") "\n", (uint8_t)s);
  }

  if (response[0] == EVT_JOINED || response[0] == EVT_JOINFAIL) {
    // callback
    if (join_done != NULL) {
      join_done((Event_code)response[0]);
    }

    if (response[0] == EVT_JOINFAIL) {
      Serial.printf(DBG_ERR("join event fail") "\n");
    }
    return false;
  }

  return true;
}

bool LoRaWANModem::is_joining() {
  return is_joining(NULL);
}

/*
 * send join command and wait for network
 * TODO sleep instead of idling 500ms!
 * TODO if join fails we get back to the modem stuck with "0x05 busy"
 */
Status LoRaWANModem::join(const uint8_t *appeui, const uint8_t *appkey) {
  // check status, if joined -> do nothing
  // avoid issues with multiple joins -> seems to create 0x05 busy errors
  uint8_t st[1] = {0};
  uint8_t sl;
  command(CMD_GETSTATUS, st, &sl);
  // Serial.printf("status: 0x%02x\n", st[0]);
  if ((Modem_status)st[0] == JOINED) {
    Serial.println(DBG_OK("already joined"));
    return OK;
  }

  Status s = command_join(appeui, appkey);
  if (s != OK) {
    Serial.printf(DBG_ERR("join request error: 0x%02x" "\n"), s);
    return FAIL;
  }

  Serial.print("waiting");
  uint8_t len;
  uint8_t response[3] = {0};

  unsigned long current_time = millis();
  while (true) {
    if ((millis()-current_time) > 500) {
      s = command(CMD_GETEVENT, response, &len);
      if (s != OK) {
        Serial.printf(DBG_ERR("pulling join event error: 0x%02x") "\n", (uint8_t)s);
      }

      if (response[0] == EVT_JOINED) {
        Serial.println(DBG_OK("joined"));
        return OK;
      }

      if (response[0] == EVT_JOINFAIL) {
        Serial.println(DBG_ERR("failed"));
        return FAIL;
      }

      current_time = millis();
      Serial.print(".");
    }
  }
}

Status LoRaWANModem::send(const uint8_t *data, uint8_t len, uint8_t port, uint8_t confirm) {
  uint8_t payload_len = len + 2;
  uint8_t payload[255];

  payload[0] = port;
  payload[1] = confirm;
  for (uint8_t i=0; i<len; i++) {
    payload[2+i] = data[i];
  }

  Status sw = _write(CMD_REQUESTTX, payload, payload_len);
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

/*
 * single command with arguments
 * get response
 */
Status LoRaWANModem::command(Lora_cmd cmd, const uint8_t *payload, uint8_t len_payload, uint8_t *response, uint8_t *len_response) {
  Status sw = _write(cmd, payload, len_payload);
  if (sw != OK) return sw;

  if (response == NULL) {
    uint8_t r[255] = {0};
    uint8_t l;
    return _read(r, &l);
  }

  return _read(response, len_response);
}

/*
 * single command no arguments
 * get response
 */
Status LoRaWANModem::command(Lora_cmd cmd, uint8_t *response, uint8_t *len_response) {
  return command(cmd, NULL, 0, response, len_response);
}

/*
 * single command with arguments
 * ignore response
 *
 * we always need to read the result or we'll mess up communication at some point
 */
Status LoRaWANModem::command(Lora_cmd cmd, const uint8_t *payload, uint8_t len_payload) {
  return command(cmd, payload, len_payload, NULL, NULL);
}

/*
 * single command no arguments
 * ignore response
 */
Status LoRaWANModem::command(Lora_cmd cmd) {
  return command(cmd, NULL, 0, NULL, NULL);
}

Status LoRaWANModem::_write(Lora_cmd cmd) {
  return _write(cmd, NULL, 0);
}

Status LoRaWANModem::_write(Lora_cmd cmd, const uint8_t *payload, uint8_t len) {
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
  return OK;
}

Status LoRaWANModem::_read(uint8_t *payload, uint8_t *len) {
  unsigned long now = millis();
  // wait for data with 100ms timeout
  while (!uart.available()) {
    if ((millis()-now) > 100) {
      Serial.println(DBG_ERR("receive timeout"));
      return TIMEOUT;
    }
  }

  while (!uart.available()) {}
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
