#pragma once

#define BCDToDec(val) ((uint8_t) (((val) / 16 * 10) + ((val) % 16)))
#define DecToBCD(val) ((uint8_t) (((val) / 10 * 16) + ((val) % 10)))

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

struct Time
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t dayOfWeek;
    uint8_t date;
    uint8_t month;
    uint8_t year;
};

void DS3231Init();
inline void DS3231Loop();
inline Time DS3231GetTime();
inline void DS3231SetTime(uint32_t epochTime);
inline bool DS3231IsReady();