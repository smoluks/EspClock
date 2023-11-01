#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

const char *ssid = "w750";
const char *password = "Mk1637gsx!";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

enum WIFIState_e
{
    WIFI_IDLE,
    WIFI_CONNECTING,
    WIFI_CONNECTED
} WIFIState = WIFI_IDLE;

void WIFIProcess()
{
    switch (WIFIState)
    {
    case WIFI_IDLE:
        WiFi.begin(ssid, password);
        WIFIState = WIFI_CONNECTING;
        break;

    case WIFI_CONNECTING:
        if (WiFi.status() == WL_CONNECTED)
        {
            WIFIState = WIFI_CONNECTED;
            ESP_LOGI("WiFi", "WiFi connected as client %s with ip %s", ssid, WiFi.localIP().toString());

            timeClient.begin();
            timeClient.setTimeOffset(3600);
            if(timeClient.update())
            {
                ESP_LOGI("WiFi", "NTP time: %d", timeClient.getEpochTime());

                DS3231SetTime(timeClient.getEpochTime());
            }     
        }
        break;

    default:
        break;
    }
}
