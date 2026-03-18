#include "h/NTPClient.hpp"
#include <WiFiUdp.h>

static const char *NTP_CLIENT_TAG = "NTPClient";

void NTPClient::init()
{
  ESP_LOGI(NTP_CLIENT_TAG, "Init started");

  _port = NTP_DEFAULT_LOCAL_PORT;

  _udp = new WiFiUDP();
  _udp->begin(_port);

  ESP_LOGI(NTP_CLIENT_TAG, "Init completed");
}

void NTPClient::sendRequest()
{
  ESP_LOGI(NTP_CLIENT_TAG, "Update from NTP Server");

  // flush any existing packets
  while (_udp->parsePacket() != 0)
    _udp->flush();

  this->sendNTPPacket();
}

uint32_t NTPClient::getResult()
{
  return _result;
}

bool NTPClient::loop()
{
  if (_udp->parsePacket() == 0)
    return false;

  int size = _udp->read(this->_packetBuffer, NTP_PACKET_SIZE);
  if (size != NTP_PACKET_SIZE)
  {
    ESP_LOGE(NTP_CLIENT_TAG, "Received packet size %d, expected %d", size, NTP_PACKET_SIZE);
    return false; // not enough data received
  }

  uint16_t highWord = word(_packetBuffer[40], _packetBuffer[41]);
  uint16_t lowWord = word(_packetBuffer[42], _packetBuffer[43]);
  // combine the four bytes (two words) into a long integer
  // this is NTP time (seconds since Jan 1 1900):
  uint32_t secsSince1900 = highWord << 16 | lowWord;

  _result = secsSince1900 - SEVENZYYEARS;
  return true;
}

void NTPClient::sendNTPPacket()
{
  // set all bytes in the buffer to 0
  memset(this->_packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  this->_packetBuffer[0] = 0b11100011; // LI, Version, Mode
  this->_packetBuffer[1] = 0;          // Stratum, or type of clock
  this->_packetBuffer[2] = 6;          // Polling Interval
  this->_packetBuffer[3] = 0xEC;       // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  this->_packetBuffer[12] = 49;
  this->_packetBuffer[13] = 0x4E;
  this->_packetBuffer[14] = 49;
  this->_packetBuffer[15] = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  if (this->_poolServerName)
  {
    this->_udp->beginPacket(this->_poolServerName, 123);
  }
  else
  {
    this->_udp->beginPacket(this->_poolServerIP, 123);
  }
  this->_udp->write(this->_packetBuffer, NTP_PACKET_SIZE);
  this->_udp->endPacket();
}
