#include "uRTCLib.h"

uRTCLib rtc(0x68);

void DS3231Init()
{
    ESP_LOGI("ds3231", "ds3231 init started");

    rtc.set_model(URTCLIB_MODEL_DS3231);

    //rtc.enableBattery();

    ESP_LOGI("ds3231", "ds3231 init completed");
}

void ds3231SetTime(uint32_t epochTime)
{
    rtc.refresh();

    rtc.set(epochTime % 60, (epochTime % 3600) / 60, (epochTime % 86400L) / 3600, rtc.dayOfWeek(), rtc.day(), rtc.month(), rtc.year());
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