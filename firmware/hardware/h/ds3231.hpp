#pragma once

struct DateTime
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
void DS3231Loop();
void DS3231SetTime(DateTime value);
DateTime DS3231GetTime();
bool DS3231TimeIsReady();