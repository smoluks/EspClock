#include "../hardware/h/btHomeV2.hpp"
#include "h/externalSensors.hpp"
#include "../h/settings.hpp"

static const char *EXTERNAL_SENSORS_TAG = "ExternalSensors";

extern settings_t settings;

void temperatureCallback(float value)
{
    ESP_LOGV(EXTERNAL_SENSORS_TAG, "t %.2f", value);

    ExternalTemperature = value;
    ExternalTemperaturePresent = true;
}

void humidityCallback(float value)
{
    ESP_LOGV(EXTERNAL_SENSORS_TAG, "h %.2f%", value);

    ExternalHumidity = value;
    ExternalHumidityPresent = true;
}

void rssiCallback(int8_t value)
{
    ESP_LOGV(EXTERNAL_SENSORS_TAG, "RSSI %d", value);

    ExternalRSSI = value;
    ExternalRSSIPresent = true;
}

void batteryCallback(int8_t value)
{
    ESP_LOGV(EXTERNAL_SENSORS_TAG, "battery %d%", value);

    ExternalBatteryLevel = value;
    ExternalBatteryLevelPresent = true;
}

void connectCallback(bool value)
{
    ESP_LOGV(EXTERNAL_SENSORS_TAG, "connected: %d", value);

    ExternalConnected = value;
}

void ExternalSensorsInit()
{
    if(settings.bthome_enable)
        BTHomeInit(temperatureCallback, humidityCallback, rssiCallback, batteryCallback, connectCallback);
}

void ExternalSensorsLoop()
{
    if(settings.bthome_enable)
        BTHomeLoop();
}