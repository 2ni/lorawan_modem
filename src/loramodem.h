#ifndef __LORA_MODEM__
#define __LORA_MODEM__

#include <Arduino.h>

#define LORA_TX       SERIAL1_TX  // PA10 U1RX
#define LORA_RX       SERIAL1_RX  // PA9 U1TX

#ifdef ANSI_DBG_OUTPUT
  #define DBG_ERR(str) "\033[31;1m" str "\033[0m"  // output in red
  #define DBG_OK(str)  "\033[32;1m" str "\033[0m"  // output in green
#else
  #define DBG_ERR(str) str
  #define DBG_OK(str)  str
#endif

typedef enum {
  OK = 0x00,
  UNKNOWN = 0x01,
  NOTIMPL = 0x02,
  NOTINIT = 0x03,
  INVALID = 0x04,
  BUSY    = 0x05,
  FAIL    = 0x06,
  BADFMT  = 0x07,
  BADCRC  = 0x08,
  BADSIG  = 0x09,
  BADSIZE = 0x0a,
  NOSESS  = 0x0b,
  FRAME   = 0x0f,
  TIMEOUT = 0xff
} Status;

typedef enum {
  BROWNOUT = 0x01,
  CRASH    = 0x02,
  MUTE     = 0x04,
  JOINED   = 0x08,
  SUSPEND  = 0x10,
  UPLOAD   = 0x20,
  JOINING  = 0x40,
  STREAM   = 0x80
} Modem_status;

typedef enum {
  EVT_RESET            = 0x00,
  EVT_ALARM            = 0x01,
  EVT_JOINED           = 0x02,
  EVT_TXDONE           = 0x03,
  EVT_DOWNDATA         = 0x04,
  EVT_UPLOADDONE       = 0x05,
  EVT_SETCONF          = 0x06,
  EVT_MUTE             = 0x07,
  EVT_STREAMDONE       = 0x08,
  EVT_LINKSTATUS       = 0x09,
  EVT_JOINFAIL         = 0x0A
} Event_code;

typedef enum {
  CMD_GETEVENT         = 0x00,
  CMD_GETVERSION       = 0x01,
  CMD_RESET            = 0x02,
  CMD_FACTORYRESET     = 0x03,
  CMD_RESETCHARGE      = 0x04,
  CMD_GETCHARGE        = 0x05,
  CMD_GETTXPOWEROFFSET = 0x06,
  CMD_SETTXPOWEROFFSET = 0x07,
  CMD_TEST             = 0x08,
  CMD_FIRMWAREUPDATE   = 0x09,
  CMD_GETTIME          = 0x0A,
  CMD_GETSTATUS        = 0x0B,
  CMD_SETALARMTIMER    = 0x0C,
  CMD_GETTRACE         = 0x0D,
  CMD_GETPIN           = 0x0E,
  CMD_GETCHIPEUI       = 0x0F,
  CMD_GETJOINEUI       = 0x10,
  CMD_SETJOINEUI       = 0x11,
  CMD_GETDEVEUI        = 0x12,
  CMD_SETDEVEUI        = 0x13,
  CMD_SETNWKKEY        = 0x14,
  CMD_GETCLASS         = 0x15,
  CMD_SETCLASS         = 0x16,
  CMD_SETMULTICAST     = 0x17,
  CMD_GETREGION        = 0x18,
  CMD_SETREGION        = 0x19,
  CMD_LISTREGIONS      = 0x1A,
  CMD_GETADRPROFILE    = 0x1B,
  CMD_SETADRPROFILE    = 0x1C,
  CMD_GETDMPORT        = 0x1D,
  CMD_SETDMPORT        = 0x1E,
  CMD_GETDMINFOINTERVAL = 0x1F,
  CMD_SETDMINFOINTERVAL = 0x20,
  CMD_GETDMINFOFIELDS  = 0x21,
  CMD_SETDMINFOFIELDS  = 0x22,
  CMD_SENDDMSTATUS     = 0x23,
  CMD_SETAPPSTATUS     = 0x24,
  CMD_JOIN             = 0x25,
  CMD_LEAVENETWORK     = 0x26,
  CMD_SUSPENDMODEMCOMM = 0x27,
  CMD_GETNEXTTXMAXPAYLOAD = 0x28,
  CMD_REQUESTTX        = 0x29,
  CMD_EMERGENCYTX      = 0x2A,
  CMD_UPLOADINIT       = 0x2B,
  CMD_UPLOADDATA       = 0x2C,
  CMD_UPLOADSTART      = 0x2D,
  CMD_STREAMINIT       = 0x2E,
  CMD_SENDSTREAMDATA   = 0x2F,
  CMD_STREAMSTATUS     = 0x30,
  CMD_GETBUDHAMODE     = 0x31,
  CMD_SETBUDHAMODE     = 0x32,
  CMD_GETBUDHACONF     = 0x33,
  CMD_SETBUDHACONF     = 0x34,
  CMD_GETDEVICEINFO    = 0x35,
  CMD_SETDEVICEINFO    = 0x36
} Lora_cmd;

class LoRaWANModem {
  public:
    LoRaWANModem(uint8_t pin_cts = 2, uint8_t pin_rts = 3);
    void begin();
    Status command_join(const uint8_t *appeui, const uint8_t *appkey);
    bool is_joining(void (*callback)(Event_code code));
    bool is_joining();
    Status join(const uint8_t *appeui, const uint8_t *appkey);
    Status send(const uint8_t *data, uint8_t len, uint8_t port = 0x01, uint8_t confirm = 0x00);

    Status command(Lora_cmd cmd, const uint8_t *payload, uint8_t len_payload, uint8_t *response, uint8_t *len_response);
    Status command(Lora_cmd cmd, uint8_t *response, uint8_t *len_response);
    Status command(Lora_cmd cmd, const uint8_t *payload, uint8_t len_payload);
    Status command(Lora_cmd cmd);
    void info();
    void cmd_and_result(const char *name, Lora_cmd cmd);
    void cmd_and_result(const char *name, Lora_cmd cmd, const uint8_t *payload, uint8_t len_payload);
    void print_arr(uint8_t *arr, uint8_t len);

  private:
    Status _write(Lora_cmd cmd);
    Status _write(Lora_cmd cmd, const uint8_t *payload, uint8_t len);
    Status _read(uint8_t *payload, uint8_t *len);
    uint8_t _calc_crc(uint8_t cmd, const uint8_t *payload, uint8_t len);
    UART uart;
    uint8_t _pin_cts;
    uint8_t _pin_rts;
};


#endif
