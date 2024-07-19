#pragma once

enum ap_enable_e
{
    AP_WHEN_NOT_CONNECTED,
    AP_ENABLE,
    AP_DISABLE
};

typedef struct settings_s
{
    //light 
    bool light_auto = true;
    bool light_level = 128; //15 - 255

    //wifi
    bool client_enable = true;
    char client_ssid[32] = "w750";
    char client_password[32] = "Mk1637gsx!";
    enum ap_enable_e ap_enable = AP_WHEN_NOT_CONNECTED;
    char ap_ssid[32] = "wifi_clock";
    char ap_password[32] = "";

    //NTP
    bool ntp_enable = true;
    uint16_t ntp_timezone_offset = 7200; //in seconds
} settings_t;
