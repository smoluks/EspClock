#include "uRTCLib.h"
#include "h/hub75.hpp"
#include "../helpers/h/unixTimeConverter.hpp"

uRTCLib rtc(0x68);

void DS3231Init()
{
    ESP_LOGI("DS3231", "DS3231 init started");

    rtc.set_model(URTCLIB_MODEL_DS3231);

    //rtc.enableBattery();

    Hub75MoveLoadingBar();
    
    ESP_LOGI("DS3231", "DS3231 init completed");
}

void DS3231SetTime(uint32_t epochTime)
{
    DateTime currentTime = UnixTimeToDateTime(epochTime);

    if(!currentTime.success)
        return;

    rtc.set(currentTime.second,
            currentTime.minute,
            currentTime.hour,
            currentTime.day + 1,
            currentTime.date,
            currentTime.month,
            currentTime.year % 100
            );

    ESP_LOGI("DS3231", "Set time: %d.%d.%d %d %d:%d:%d", currentTime.date, currentTime.month, currentTime.year, currentTime.day+1, currentTime.hour, currentTime.minute, currentTime.second);
}

inline void DS3231RefreshTime()
{
    rtc.refresh();
}

inline uint8_t DS3231GetMinute()
{
    return rtc.minute();
}

inline uint8_t DS3231GetHour()
{
    return rtc.hour();
}