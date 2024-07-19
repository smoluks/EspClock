#include <WiFiUdp.h>
#include <NTPClient.h>
#include "h/clockController.hpp"
#include "../helpers/h/unixTimeConverter.hpp"
#include "../h/settings.hpp"

extern settings_t settings;

inline bool IsTimePresent()
{
    return DS3231TimeIsReady();
}

inline DateTime GetCurrentTime()
{
    return DS3231GetTime();
} 

inline void SetCurrentTime(DateTime time)
{
    DS3231SetTime(time);
} 

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
void UpdateTimeFromNTP()
{
    timeClient.begin();
    timeClient.setTimeOffset(settings.ntp_timezone_offset);
    if (timeClient.update())
    {
        ESP_LOGI("WiFi", "NTP time: %d", timeClient.getEpochTime());

        DateTime result;
        if (TryConvertUnixTimeToDateTime(timeClient.getEpochTime(), &result))
        {
            SetCurrentTime(result);
        }
    }
}
