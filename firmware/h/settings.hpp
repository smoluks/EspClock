#pragma once

typedef struct settings_s
{
    //light 
    bool light_auto = true;
    bool light_level = 128; //15 - 255

    //wifi
    bool client_enable = true;
    char client_ssid[32] = "w750";
    char client_password[32] = "Mk1637gsx!";

    //NTP
    bool ntp_enable = true;
    uint16_t ntp_timezone_offset = 3600; //in seconds

    //BTHome
    uint8_t bthome_enable = true;
    uint8_t bthome_sensor_addr[6] = {0x38, 0x1F, 0x8D, 0xFC, 0x36, 0x1D};
} settings_t;
