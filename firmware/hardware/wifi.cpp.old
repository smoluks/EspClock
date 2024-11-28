#include <WiFi.h>
#include <esp_wifi.h>

//#include "../controllers/h/webController.hpp"
#include "../h/settings.hpp"
#include "h/WiFi.hpp"

static const char *WIFI_TAG = "WiFi";

void setupAP();
void setupClient();
void UpdateTimeFromNTP();
inline void ProcessStatus(wl_status_t newWifiStatus);

static const IPAddress localIP(4, 3, 2, 1);   // the IP address the web server, Samsung requires the IP to be in public space
static const IPAddress gatewayIP(4, 3, 2, 1); // IP address of the network should be the same as the local IP for captive portals
static const IPAddress subnetMask(255, 255, 255, 0);
static const String localIPURL = "http://4.3.2.1";
static wl_status_t wifiStatus = WL_IDLE_STATUS;

extern settings_t settings;

inline void WiFiInit()
{
    ESP_LOGI(WIFI_TAG, "WiFi init started");

    /*
    // Disable AMPDU RX on the ESP32 WiFi to fix a bug on Android
    esp_wifi_stop();
    esp_wifi_deinit();
    wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();
    my_config.ampdu_rx_enable = false;
    esp_wifi_init(&my_config);
    esp_wifi_start();
    vTaskDelay(100 / portTICK_PERIOD_MS); // Add a small delay*/

    WiFi.setHostname("wifi.clock");

    if (settings.client_enable)
    {
        setupClient();
    }

    if (settings.ap_enable == AP_ENABLE)
        setupAP();

    ESP_LOGI(WIFI_TAG, "WiFi init finished");
}

void setupClient()
{
    wl_status_t wifiStatus = WiFi.begin(settings.client_ssid, settings.client_password);
    ProcessStatus(wifiStatus);
}

void setupAP()
{
    WiFi.softAPConfig(localIP, gatewayIP, subnetMask);
    WiFi.softAP(settings.ap_ssid, settings.ap_password);
    WiFi.enableAP(true);

    //WebServerSetup();
}

inline void WiFiLoop()
{
    wl_status_t newWifiStatus = WiFi.status();
    if (newWifiStatus != wifiStatus)
    {
        wifiStatus = newWifiStatus;
        ProcessStatus(newWifiStatus);
    }
}

inline void ProcessStatus(wl_status_t newWifiStatus)
{
    switch (wifiStatus)
    {
    case WL_CONNECTED:
        ESP_LOGI(WIFI_TAG, "WiFi connected as client %s with ip %s", settings.client_ssid, WiFi.localIP().toString());
        if (settings.ntp_enable)
            UpdateTimeFromNTP();
        break;
    case WL_NO_SSID_AVAIL:
        ESP_LOGI(WIFI_TAG, "No SSID available");
        break;
    case WL_CONNECT_FAILED:
        ESP_LOGI(WIFI_TAG, "Connect failed");
        break;
    //-----it's auto reconnected-----
    case WL_CONNECTION_LOST:
        ESP_LOGI(WIFI_TAG, "Connection lost");
        break;
        //WiFi.enableSTA(false);
        //if (settings.ap_enable == AP_WHEN_NOT_CONNECTED)
        //    WiFi.enableAP(false);
        //break;
    case WL_DISCONNECTED:
        ESP_LOGI(WIFI_TAG, "Disconnected");
        break;
        //if (settings.ap_enable == AP_WHEN_NOT_CONNECTED)
        //    setupAP();
        //break;       

    default:
        break;
    }
}
