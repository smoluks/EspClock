#include "h/i2c.hpp"
#include "../helpers/h/unixTimeConverter.hpp"
#include "../managers/h/errorManager.hpp"

#define BCDToDec(val) ((uint8_t)(((val) / 16 * 10) + ((val) % 16)))
#define DecToBCD(val) ((uint8_t)(((val) / 10 * 16) + ((val) % 10)))

#define DS3231_I2C_ADDR 0x68

#define DS3231_REG_CONTROL 0x0E
#define DS3231_REG_CONTROL_CONV (1 << 5)
#define DS3231_REG_STATUS 0x0F

DateTime DS3231_current_time;
bool DS3231_time_is_ready = false;
bool DS3231_need_update_time;
DateTime DS3231_time_to_update;

bool trySetTime(DateTime time);
void readTime();
uint8_t readSeconds();

void DS3231Init()
{
    ESP_LOGI("DS3231", "DS3231 init started");

    // trying to read
    uint8_t value;
    if (I2CReadRegister(DS3231_I2C_ADDR, DS3231_REG_STATUS, &value) != 1)
    {
        setError(ERROR_DS3231_NOT_FOUND);
        ESP_LOGE("DS3231", "DS3231 not answered");
        return;
    }
    ESP_LOGI("DS3231", "DS3231 status 0x%X", value);

    I2CWriteRegister(DS3231_I2C_ADDR, DS3231_REG_CONTROL, 0);

    // Start temperature calibration
    // if(!(value & DS3231_REG_CONTROL_CONV))
    //     I2CWriteRegister(DS3231_I2C_ADDR, DS3231_REG_CONTROL, DS3231_REG_CONTROL_CONV);
    // else
    //     I2CWriteRegister(DS3231_I2C_ADDR, DS3231_REG_CONTROL, 0);

    // Wait temperature calibration
    // do
    //{
    //     I2CReadRegister(DS3231_I2C_ADDR, DS3231_REG_CONTROL, &value);
    // } while (value & DS3231_REG_CONTROL_CONV);

    readTime();

    ESP_LOGI("DS3231", "DS3231 init finished");
}

uint8_t raw_seconds;
void DS3231Loop()
{
    if (DS3231_need_update_time)
    {
        uint8_t try_count = 0;
        while (try_count++ < 3)
        {
            if (trySetTime(DS3231_time_to_update))
            {
                DS3231_need_update_time = false;
                break;
            }

            delay(5);
        }
    }

    if (readSeconds() != raw_seconds)
        readTime();
}

void DS3231SetTime(DateTime value)
{
    DS3231_time_to_update = value;
    DS3231_need_update_time = true;
}

DateTime DS3231GetTime()
{
    return DS3231_current_time;
}

bool DS3231TimeIsReady()
{
    return DS3231_time_is_ready;
}

void readTime()
{
    uint8_t result[7];
    if (I2CReadRegisters(DS3231_I2C_ADDR, 0, result, sizeof(result)) != sizeof(result))
    {
        setError(ERROR_DS3231_NOT_FOUND);
        ESP_LOGE("DS3231", "DS3231 not answered");
        return;
    }

    DS3231_current_time.second = BCDToDec(result[0]);
    DS3231_current_time.minute = BCDToDec(result[1]);
    DS3231_current_time.hour = BCDToDec(result[2]);
    DS3231_current_time.dayOfWeek = BCDToDec(result[3]);
    DS3231_current_time.date = BCDToDec(result[4]);
    DS3231_current_time.month = BCDToDec(result[5] & 0x1F);
    DS3231_current_time.year = BCDToDec(result[6]);
    DS3231_time_is_ready = true;

    raw_seconds = result[0];
    ESP_LOGV("DS3231", "DS3231 time %X:%X:%X", result[2], result[1], result[0]);
}

uint8_t readSeconds()
{
    uint8_t result;
    if (I2CReadRegister(DS3231_I2C_ADDR, 0, &result) != 1)
    {
        setError(ERROR_DS3231_NOT_FOUND);
        ESP_LOGE("DS3231", "DS3231 not answered");
        return 255;
    }

    return result;
}

bool trySetTime(DateTime time)
{
    uint8_t command[7] = {
        DecToBCD(time.second),
        DecToBCD(time.minute),
        DecToBCD(time.hour),
        DecToBCD(time.dayOfWeek + 1),
        DecToBCD(time.date),
        0B10000000 | DecToBCD(time.month + 1),
        DecToBCD(time.year % 100)};

    if (I2CWriteRegisters(DS3231_I2C_ADDR, 0, command, sizeof(command)) == sizeof(command))
    {
        ESP_LOGI("DS3231", "Set time: %d.%d.%d %d %d:%d:%d", time.date, time.month + 1, time.year, time.dayOfWeek + 1, time.hour, time.minute, time.second);
        return true;
    }
    else
    {
        ESP_LOGW("DS3231", "DS3231 time write error, time: %d.%d.%d %d %d:%d:%d", time.date, time.month + 1, time.year, time.dayOfWeek + 1, time.hour, time.minute, time.second);
        return false;
    }
}
