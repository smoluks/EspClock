#include "h/hub75.hpp"
#include "h/i2c.hpp"
#include "../helpers/h/unixTimeConverter.hpp"
#include "../managers/h/errorManager.hpp"

#define DS3231_I2C_ADDR 0x68

Time _currentTime;
uint32_t _NTPTime = 0;
bool _timeIsReady = false;

void setTime(uint32_t epochTime);
void updateTime();

void DS3231Init()
{
    uint8_t value;
    if(I2CReadRegister(DS3231_I2C_ADDR, 0, &value) != 1)
    {
        setError(ERROR_DS3231_NOT_FOUND);
        ESP_LOGE("DS3231", "DS3231 not answered");        
        return;
    }

    //Start temperature calibration
    if(value & 0b00100000 == 0)
        I2CWriteRegister(DS3231_I2C_ADDR, 0x0E, 0b00100000);
    else
        I2CWriteRegister(DS3231_I2C_ADDR, 0x0E, 0);

    //Wait temperature calibration
    do
    {
        I2CReadRegister(DS3231_I2C_ADDR, 0x0E, &value);
    } while (value & 0b00100000);

    updateTime();
}

inline void DS3231Loop()
{
    if(_NTPTime)
    {
        setTime(_NTPTime);
        _NTPTime = 0;
    }

    updateTime();
}

inline Time DS3231GetTime()
{
    return _currentTime;
}

inline void DS3231SetTime(uint32_t epochTime)
{
    _NTPTime = epochTime;
}

inline bool DS3231IsReady()
{
    return _timeIsReady;
}

void updateTime()
{
    uint8_t result[7];
    if(I2CReadRegisters(DS3231_I2C_ADDR, 0, result, sizeof(result)) != sizeof(result))
    {
        setError(ERROR_DS3231_NOT_FOUND);
        ESP_LOGE("DS3231", "DS3231 not answered");
        return;
    }
    
    _currentTime.second = BCDToDec(result[0]);
	_currentTime.minute = BCDToDec(result[1]);
	_currentTime.hour = BCDToDec(result[2]);
    _currentTime.dayOfWeek = BCDToDec(result[3]);
    _currentTime.date = BCDToDec(result[4]);
	_currentTime.month = BCDToDec(result[5] & 0x1F);
    _currentTime.year = BCDToDec(result[6]);
    _timeIsReady = true;

    ESP_LOGV("DS3231", "DS3231 time %X:%X:%X", result[2], result[1], result[0]);
}

void setTime(uint32_t epochTime)
{
    DateTime currentTime = UnixTimeToDateTime(epochTime);

    if(!currentTime.success)
        return;

    uint8_t command[7] = {
            DecToBCD(currentTime.second),
            DecToBCD(currentTime.minute),
            DecToBCD(currentTime.hour),
            DecToBCD(currentTime.day + 1),
            DecToBCD(currentTime.date),
            0B10000000 | DecToBCD(currentTime.month + 1),
            DecToBCD(currentTime.year % 100)
    };

    if(I2CWriteRegisters(DS3231_I2C_ADDR, 0, command, sizeof(command)) == sizeof(command))
        ESP_LOGI("DS3231", "Set time: %d.%d.%d %d %d:%d:%d", currentTime.date, currentTime.month + 1, currentTime.year, currentTime.day+1, currentTime.hour, currentTime.minute, currentTime.second);
    else
        ESP_LOGW("DS3231", "DS3231 time write error");
}
