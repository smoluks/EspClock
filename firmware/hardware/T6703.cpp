#include <T67XX.h>

T67XX co2sensor;
bool T6703Present = false;
void T6703Init()
{
    ESP_LOGI("T6703", "T6703 init started");

    if (!co2sensor.begin())
    {
        ESP_LOGW("T6703", "T6703 init error");
    }

    co2sensor.reset();

    uint8_t tryCount = 0;
    uint16_t sensorStatus = co2sensor.getStatus();
    while (sensorStatus && tryCount < 10)
    {
        ESP_LOGI("T6703", "T6703 status: %d", sensorStatus);

        delay(T67XX_MEASURE_DELAY);
        sensorStatus = co2sensor.getStatus();
        tryCount++;
    }
    if(sensorStatus)
    {
        return;
    }
    T6703Present = true;

    co2sensor.setABCMode(true);
    co2sensor.flashUpdate();

    ESP_LOGI("T6703", "Sensor firmware version: %d", co2sensor.getFirmwareVersion());
    ESP_LOGI("T6703", "T6703 init finished at %d cycles", tryCount);
}

bool T6703IsPresent()
{
    return T6703Present;
}

uint16_t T6703GetCO2()
{
    return co2sensor.readPPM();
}