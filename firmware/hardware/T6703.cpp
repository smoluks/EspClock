#include <T67XX.h>

enum T6703State_e
{
    T6703_IDLE,
    T6703_INITIALISING,
    T6703_INITIALISED
} T6703State = T6703_IDLE;

T67XX co2sensor;
void T6703Init()
{
    ESP_LOGI("T6703", "T6703 init started");

    URTCLIB_WIRE.setClock(100000);

    if (!co2sensor.begin())
    {
        ESP_LOGW("T6703", "T6703 init error");
    }
    else
    {
        T6703State = T6703_INITIALISING;
        co2sensor.reset();
    }

    URTCLIB_WIRE.setClock(400000);
}

void T6703Loop()
{
    switch (T6703State)
    {
    case T6703_INITIALISING:
        URTCLIB_WIRE.setClock(100000);

        uint16_t sensorStatus = co2sensor.getStatus();
        if (!sensorStatus)
        {
            ESP_LOGI("T6703", "T6703 status: %d", sensorStatus);
        }
        else
        {
            co2sensor.setABCMode(true);
            co2sensor.flashUpdate();
            T6703State = T6703_INITIALISED;

            ESP_LOGI("T6703", "Sensor firmware version: %d", co2sensor.getFirmwareVersion());
        }

        URTCLIB_WIRE.setClock(400000);
        break;
    }
}

bool T6703IsPresent()
{
    return T6703State == T6703_INITIALISED;
}

uint16_t T6703GetCO2()
{
    if (T6703State != T6703_INITIALISED)
    {
        return 0;
    }

    URTCLIB_WIRE.setClock(100000);

    uint16_t result = co2sensor.readPPM();

    URTCLIB_WIRE.setClock(400000);

    return result;
}