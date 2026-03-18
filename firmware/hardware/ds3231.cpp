#include <controllers/h/clock.hpp>
#include <hardware/h/i2c.hpp>
#include <managers/h/errorManager.hpp>

static const char *DS3231_TAG = "DS3231";

static bool trySetTime(DateTime time);
static void readTime();
static uint8_t readSeconds();

static bool needUpdateTime;
static DateTime newTime;

void DS3231Init()
{
    ESP_LOGI(DS3231_TAG, "DS3231 init started");

    // trying to read
    uint8_t value;
    if (!I2CReadRegister(DS3231_I2C_ADDR, DS3231_REG_STATUS, &value))
    {
        setError(ERROR_DS3231_NOT_FOUND);
        ESP_LOGE(DS3231_TAG, "DS3231 not answered");
        return;
    }
    ESP_LOGI(DS3231_TAG, "DS3231 status 0x%X", value);

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

    ESP_LOGI(DS3231_TAG, "DS3231 init finished");
}

uint8_t raw_seconds;
void DS3231Loop()
{
    if (needUpdateTime)
    {
        trySetTime(newTime);
        needUpdateTime = false;
    }

    if (readSeconds() != raw_seconds)
        readTime();
}

void DS3231UpdateTime(DateTime value)
{
    newTime = value;
    needUpdateTime = true;
}

void readTime()
{
    uint8_t result[7];
    if (!I2CReadRegisters(DS3231_I2C_ADDR, 0, result, sizeof(result)))
    {
        setError(ERROR_DS3231_NOT_FOUND);
        ESP_LOGE(DS3231_TAG, "DS3231 not answered");
        return;
    }
    DateTime currentTime;
    currentTime.second = BCDToDec(result[0]);
    currentTime.minute = BCDToDec(result[1]);
    currentTime.hour = BCDToDec(result[2]);
    currentTime.dayOfWeek = BCDToDec(result[3]);
    currentTime.date = BCDToDec(result[4]);
    currentTime.month = BCDToDec(result[5] & 0x1F);
    currentTime.year = BCDToDec(result[6]);

    TimeChangedHandler(currentTime);

    raw_seconds = result[0];

    ESP_LOGV(DS3231_TAG, "DS3231 time %X:%X:%X", result[2], result[1], result[0]);
}

uint8_t readSeconds()
{
    uint8_t result;
    if (!I2CReadRegister(DS3231_I2C_ADDR, 0, &result))
    {
        setError(ERROR_DS3231_NOT_FOUND);
        ESP_LOGE(DS3231_TAG, "DS3231 not answered");
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

    if (I2CWriteRegisters(DS3231_I2C_ADDR, 0, command, sizeof(command)))
    {
        ESP_LOGI(DS3231_TAG, "Set time: %d.%d.%d %d %d:%d:%d", time.date, time.month + 1, time.year, time.dayOfWeek + 1, time.hour, time.minute, time.second);
        return true;
    }
    else
    {
        ESP_LOGW(DS3231_TAG, "DS3231 time write error, time: %d.%d.%d %d %d:%d:%d", time.date, time.month + 1, time.year, time.dayOfWeek + 1, time.hour, time.minute, time.second);
        return false;
    }
}
