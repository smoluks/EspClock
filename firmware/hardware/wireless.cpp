#include <WiFi.h>
#include "../libs/pt-1.4/pt.h"
#include "../h/settings.hpp"
#include "h/BTHomeV2.hpp"

static const char *WIRELESS_TAG = "Wireless";

void setupAP();
void setupClient();
void UpdateTimeFromNTP();
inline void ProcessStatus(wl_status_t newWifiStatus);
bool IsConnectFinished();

extern settings_t settings;

void WirelessInit()
{
    ESP_LOGI(WIRELESS_TAG, "WiFi started");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.begin(settings.client_ssid, settings.client_password);
}

bool IsConnectFinished()
{
    wl_status_t status = WiFi.status();

    switch (status)
    {
    case WL_DISCONNECTED:
    case WL_IDLE_STATUS:
        return false;
    case WL_CONNECTED:
        ESP_LOGI(WIRELESS_TAG, "WiFi connected as client %s with ip %s, RSSI %d", settings.client_ssid, WiFi.localIP().toString(), WiFi.RSSI());
        if (settings.ntp_enable)
            UpdateTimeFromNTP();
        return true;
    case WL_NO_SSID_AVAIL:
        ESP_LOGE(WIRELESS_TAG, "No SSID available");
        return true;
    case WL_CONNECT_FAILED:
        ESP_LOGE(WIRELESS_TAG, "Connect failed");
        return true;
    default:
        ESP_LOGI(WIRELESS_TAG, "Status: %d", status);
        return false;
    }
}