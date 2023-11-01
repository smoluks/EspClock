#include <Adafruit_BME280.h>
#include "h/bme280.hpp"
#include "h/hub75.hpp"

Adafruit_BME280 bme;

void BME280Init()
{
    ESP_LOGI("BME280", "BME280 init started");

    if (!bme.begin())
    {
        ESP_LOGW("BME280", "BME280 init error");
    }
    
    Hub75MoveLoadingBar();
    
    ESP_LOGI("BME280", "BME280 init finished");
}

float BME280GetTemperature()
{
    return bme.readTemperature();
}

float BME280GetPressure()
{
    return bme.readPressure() / 100.0F;
}

float BME280GetAltitude()
{
    return bme.readAltitude(SEALEVELPRESSURE_HPA);
}

float BME280GetHumidity()
{
    return bme.readHumidity();
}