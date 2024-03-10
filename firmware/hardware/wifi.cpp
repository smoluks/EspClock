#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "h/ds3231.hpp"
#include "../h/settings.hpp"

extern settings_t settings;

wl_status_t wifiStatus = WL_IDLE_STATUS;

void updateTimeFromNTP();

inline void WiFiInit()
{    
    if(settings.client_enable)
        WiFi.begin(settings.client_ssid, settings.client_password);

    if(settings.ap_enable == AP_ENABLE)
        WiFi.softAP(settings.ap_ssid, settings.ap_password);
}

inline void WiFiLoop()
{
    //Status change handlers
    wl_status_t newWifiStatus = WiFi.status();
    if(newWifiStatus != wifiStatus)
    {
        wifiStatus = newWifiStatus;        
        ESP_LOGI("WiFi", "WiFi new status %d", newWifiStatus);

        switch(wifiStatus)
        {
            case WL_CONNECTED:
                ESP_LOGI("WiFi", "WiFi connected as client %s with ip %s", settings.client_ssid, WiFi.localIP().toString());
                if(settings.ap_enable == AP_WHEN_NOT_CONNECTED)
                    WiFi.enableAP(false);
                if(settings.ntp_enable)
                    updateTimeFromNTP();
                break;
            case WL_CONNECT_FAILED:
                if(settings.ap_enable == AP_WHEN_NOT_CONNECTED)
                    WiFi.enableAP(true);
                    WiFi.softAP(settings.ap_ssid, settings.ap_password);
                break;
        }
    }
}

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
void updateTimeFromNTP()
{ 
    timeClient.begin();
    timeClient.setTimeOffset(settings.timezone_offset);
    if (timeClient.update())
    {
        ESP_LOGV("WiFi", "NTP time: %d", timeClient.getEpochTime());

        DS3231SetTime(timeClient.getEpochTime());
    }
}
