#include <WiFi.h>
#include "../h/settings.hpp"

static const char *WIFI_TAG = "WiFi";

void setupAP();
void setupClient();
void UpdateTimeFromNTP();
inline void ProcessStatus(wl_status_t newWifiStatus);
bool IsConnectFinished();

extern settings_t settings;

void WifiInit()
{
    ESP_LOGI(WIFI_TAG, "WiFi started");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.begin(settings.client_ssid, settings.client_password);
}

wl_status_t GetWifiStatus()
{
    return WiFi.status();
}