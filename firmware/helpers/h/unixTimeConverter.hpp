#pragma once

struct DateTime
{
    bool success;
    uint8_t year;
    uint8_t month;
    uint8_t date;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

DateTime UnixTimeToDateTime(uint32_t unixTime);