#ifndef NTPClient_h
#define NTPClient_h

#include "Arduino.h"

#include <Udp.h>

#define SEVENZYYEARS 2208988800UL
#define NTP_PACKET_SIZE 48
#define NTP_DEFAULT_LOCAL_PORT 1337

class NTPClient {
  private:
    UDP*          _udp;
    const char*   _poolServerName = "pool.ntp.org"; // Default time server
    IPAddress     _poolServerIP;
    unsigned int  _port           = NTP_DEFAULT_LOCAL_PORT;
    byte          _packetBuffer[NTP_PACKET_SIZE];
    uint32_t      _result;
    void          sendNTPPacket();

  public:
    void init();
    void sendRequest();
    uint32_t getResult();
    bool loop();
};

#endif // NTPClient_h