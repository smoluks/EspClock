#include <WiFiUdp.h>
#include <network/h/NTPClient.hpp>
#include <hardware/h/ds3231.hpp>
#include <hardware/h/wifi.hpp>
#include <libs/pt-1.4/pt.h>
#include <helpers/h/unixTimeConverter.hpp>
#include <h/settings.hpp>
#include <network/h/timezonedb.hpp>

static const char *CLOCK_TAG = "Clock";

PT_THREAD(NTPPt());

extern settings_t settings;

static bool isTimePresent = false;
static bool isTimeUpdated = false;
static DateTime currentTime;
void TimeChangedHandler(DateTime value)
{
    currentTime = value;
    isTimePresent = true;
    isTimeUpdated = true;
}

static bool ntpSkip = false;
void ClockLoop()
{
    if(!settings.ntp_enable || ntpSkip)
    {
        return;
    }

    if(NTPPt() >= PT_EXITED)
    {
        ntpSkip = true;
    }
}

static struct pt clockPt;
static NTPClient timeNtpClient;
static uint32_t offset, result = 0;
PT_THREAD(NTPPt())
{  
    PT_BEGIN(&clockPt);

    PT_WAIT_UNTIL(&clockPt, settings.ntp_enable && GetWifiStatus() == WL_CONNECTED);

    if(!GetTimezoneInfo(&offset))
    {
        ESP_LOGE(CLOCK_TAG, "Failed to get timezone info");
        PT_EXIT(&clockPt);
    }

    ESP_LOGI(CLOCK_TAG, "sending NTP request");

    timeNtpClient.init();
    timeNtpClient.sendRequest();

    PT_WAIT_UNTIL(&clockPt, timeNtpClient.loop());

    ESP_LOGI(CLOCK_TAG, "parce NTP result");

    DateTime ntpTime;
    result = timeNtpClient.getResult();

    if (TryConvertUnixTimeToDateTime(result + offset, &ntpTime))
    {
        DS3231UpdateTime(ntpTime);
    }

    PT_WAIT_WHILE(&clockPt, true);

    PT_END(&clockPt);
}

inline bool IsTimePresent()
{
    return isTimePresent;
}

inline bool IsTimeUpdated()
{
    if (isTimeUpdated)
    {
        isTimeUpdated = false;
        return true;
    }
    else
        return false;
}

inline DateTime GetCurrentTime()
{
    return currentTime;
}

inline void SetCurrentTime(DateTime time)
{
    DS3231UpdateTime(time);
}