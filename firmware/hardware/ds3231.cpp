#include "Wire.h"
#include "uRTCLib.h"

uRTCLib rtc(0x68);

void DS3231Init()
{
    ESP_LOGI("ds3231", "ds3231 init started");

    if (!URTCLIB_WIRE.begin(21, 22, 400000))
    {
        Serial.print("I2C init error");
    }

    // URTCLIB_WIRE.setTimeOut(500);

    rtc.set_model(URTCLIB_MODEL_DS3231);

    ESP_LOGI("ds3231", "ds3231 init completed");
}

void ds3231SetTime()
{
    // rtc.set(0, 02, 16, 4, 21, 9, 23);
}

void ds3231RefreshTime()
{
    rtc.refresh();
}

inline uint8_t getMinute()
{
    return rtc.minute();
}

inline uint8_t getHour()
{
    return rtc.hour();
}