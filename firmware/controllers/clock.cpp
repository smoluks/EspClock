#include <WiFiUdp.h>
#include <NTPClient.h>
#include "h/clock.hpp"
#include "../helpers/h/unixTimeConverter.hpp"
#include "../h/settings.hpp"

extern settings_t settings;
extern void (*DS3231TimeChangedHandler)(DateTime value);

static bool isTimePresent = false;
static bool isTimeUpdated = false;
static DateTime current_time;
inline void timeChangedHandler(DateTime value)
{
    isTimePresent = true;
    isTimeUpdated = true;
    current_time = value;
}

inline void ClockInit()
{
    DS3231TimeChangedHandler = timeChangedHandler;
}

inline bool IsTimePresent()
{
    return isTimePresent;
}

inline bool IsTimeUpdated()
{
    if(isTimeUpdated)
    {
        isTimeUpdated = false;
        return true;
    } else return false;
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
