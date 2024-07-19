#include "h/i2c.hpp"
#include "h/T6703.hpp"

uint16_t t6703_value;
bool t6703_ready;
bool t6703_pt_finised;

PT_THREAD(T6703checkStatus());
PT_THREAD(T6703read());

inline void T6703Init()
{
    ESP_LOGI("T6703", "T6703 init started");

    I2CSetSpeed(100000);

    //-----Waiting status OK------
    int32_t status;
    bool is_resetted;
    do
    {
        status = ModbusOverI2CRead(T67XX_I2C_ADDR, T67XX_REG_STATUS);
        if(status == -1)
        {     
            //i2c operation failed
            if(is_resetted)
            {
                I2CSetSpeed(400000);
                t6703_pt_finised = true;
                return;
            }
            else
            {
                ModbusOverI2CWriteSingleCoil(T67XX_I2C_ADDR, T67XX_REG_RESET, true);
                delay(15);
                is_resetted = true;
            }
        }
    } while (status < 0);

    //-----Setup sensor------
    ModbusOverI2CWriteSingleCoil(T67XX_I2C_ADDR, T67XX_REG_ABC_LOGIC, true);

    //
    ESP_LOGI("T6703", "T6703 init completed, firmware version: %d", ModbusOverI2CRead(T67XX_I2C_ADDR, T67XX_REG_FIRMWARE));

    I2CSetSpeed(400000);
}

inline void T6703Loop()
{
    if (t6703_pt_finised)
        return;

    if (T6703read() == PT_EXITED)
    {
        t6703_pt_finised = true;
    }
}

struct pt t6703_read_pt;
uint32_t t6703_read_timestamp;
PT_THREAD(T6703read())
{
    uint8_t t6703_read_command[] = {0x04, byte(T67XX_REG_PPM >> 8), byte(T67XX_REG_PPM & 0xFF), 0x00, 0x01};
    uint8_t t6703_answer[4];

    PT_BEGIN(&t6703_read_pt);

    //-----throttle-----
    t6703_read_timestamp = millis();
    PT_WAIT_WHILE(&t6703_read_pt, (millis() - t6703_read_timestamp) < T67XX_MEASURE_DELAY);

    //-----Check status------
    PT_WAIT_UNTIL(&t6703_read_pt, T6703checkStatus() == PT_EXITED);
    ESP_LOGV("T6703", "T6703 status ok");

    //-----write measure command-----
    ESP_LOGV("T6703", "write measure command");

    I2CSetSpeed(100000);

    if (I2CWriteBulk(T67XX_I2C_ADDR, t6703_read_command, 5) != 5)
    {
        ESP_LOGW("T6703", "T6703 write co2 command failed");
        I2CSetSpeed(400000);
        PT_INIT(&t6703_read_pt);
        return PT_WAITING;
    }

    I2CSetSpeed(400000);

    //-----wait command processing-----
    ESP_LOGV("T6703", "wait command processing");

    t6703_read_timestamp = millis();
    PT_WAIT_WHILE(&t6703_read_pt, (millis() - t6703_read_timestamp) < T67XX_READ_DELAY);

    //-----read result-----
    ESP_LOGV("T6703", "read result");

    I2CSetSpeed(100000);

    if (I2CReadBulk(T67XX_I2C_ADDR, t6703_answer, 4) != 4)
    {
        ESP_LOGW("T6703", "T6703 read co2 result failed");
        I2CSetSpeed(400000);
        PT_INIT(&t6703_read_pt);
        return PT_WAITING;
    }
    t6703_value = ((t6703_answer[2] << 8) | t6703_answer[3]);
    t6703_ready = true;

    ESP_LOGV("T6703", "CO2 = %d ppm", co2);

    I2CSetSpeed(400000);

    PT_END(&t6703_read_pt);
}

struct pt t6703_status_pt;
uint32_t t6703_status_timestamp;
PT_THREAD(T6703checkStatus())
{
    uint8_t t6703_read_command[] = {0x04, byte(T67XX_REG_STATUS >> 8), byte(T67XX_REG_STATUS & 0xFF), 0x00, 0x01};
    uint8_t t6703_answer[4];

    PT_BEGIN(&t6703_status_pt);

    //-----write read status command-----
    ESP_LOGV("T6703", "write read status command");

    I2CSetSpeed(100000);
    if (I2CWriteBulk(T67XX_I2C_ADDR, t6703_read_command, 5) != 5)
    {
        ESP_LOGW("T6703", "T6703 write status command failed");
        I2CSetSpeed(400000);
        PT_INIT(&t6703_status_pt);
        return PT_WAITING;
    }
    I2CSetSpeed(400000);

    //-----wait command processing-----
    ESP_LOGV("T6703", "wait command processing");

    t6703_status_timestamp = millis();
    PT_WAIT_WHILE(&t6703_status_pt, (millis() - t6703_status_timestamp) < T67XX_READ_DELAY);

    //-----read result-----
    ESP_LOGV("T6703", "read read status result");

    I2CSetSpeed(100000);

    if (I2CReadBulk(T67XX_I2C_ADDR, t6703_answer, 4) != 4)
    {
        ESP_LOGW("T6703", "T6703 read status result failed");
        I2CSetSpeed(400000);
        PT_INIT(&t6703_status_pt);
        return PT_WAITING;
    }
    uint16_t status = ((t6703_answer[2] << 8) | t6703_answer[3]);
    ESP_LOGV("T6703", "T6703 status = %d", status);

    I2CSetSpeed(400000);

    if (!status)
        PT_EXIT(&t6703_status_pt);

    PT_END(&t6703_status_pt);
}

