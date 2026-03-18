#ifndef DS3231_HPP
#define DS3231_HPP

#define BCDToDec(val) ((uint8_t)(((val) / 16 * 10) + ((val) % 16)))
#define DecToBCD(val) ((uint8_t)(((val) / 10 * 16) + ((val) % 10)))

#define DS3231_I2C_ADDR 0x68

#define DS3231_REG_CONTROL 0x0E
#define DS3231_REG_CONTROL_CONV (1 << 5)
#define DS3231_REG_STATUS 0x0F

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
void DS3231UpdateTime(DateTime value);
DateTime DS3231GetTime();
bool DS3231TimeIsReady();

#endif // DS3231_HPP