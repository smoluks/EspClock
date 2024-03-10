#include "../hardware/h/i2c.hpp"
#include "../hardware/h/ds3231.hpp"
#include "../hardware/h/bme280.hpp"
#include "../hardware/h/fusb302.hpp"
#include "../hardware/h/T6703.hpp"

/// @brief Task for all i2c bus work
void i2cTask(void *parameter)
{
    uint32_t i2c_task_loop_timestamp = 0;

    ESP_LOGV("i2cTask", "i2cTask running on core %d", xPortGetCoreID());

    //i2c_task_loop_timestamp = micros();
    I2CInit();
    //ESP_LOGI("i2cTask", "I2C init take %d uS", micros() - i2c_task_loop_timestamp);

    //i2c_task_loop_timestamp = micros();
    DS3231Init();
    //ESP_LOGI("i2cTask", "DS3231 init take %d uS", micros() - i2c_task_loop_timestamp);

    //i2c_task_loop_timestamp = micros();
    BME280Init();
    //ESP_LOGI("i2cTask", "BME280 init take %d uS", micros() - i2c_task_loop_timestamp);

    //i2c_task_loop_timestamp = micros();
    T6703Init();
    //ESP_LOGI("i2cTask", "T6703 init take %d uS", micros() - i2c_task_loop_timestamp);

    //i2c_task_loop_timestamp = micros();
    //FUSB302Init();
    //ESP_LOGI("i2cTask", "FUSB302 init take %d uS", micros() - i2c_task_loop_timestamp);
   
    for (;;)
    {
        // ESP_LOGI("i2cTask", "cycle running on core %d with interval %d", xPortGetCoreID(), micros() - i2c_task_loop_timestamp);

        //FUSB302Loop();

        //i2c_task_loop_timestamp = micros();
        BME280Loop();
        //if (micros() - i2c_task_loop_timestamp > 10000)
        //    ESP_LOGW("i2cTask", "BME280 read take %d uS", micros() - i2c_task_loop_timestamp);

        //FUSB302Loop();

        //i2c_task_loop_timestamp = micros();
        DS3231Loop();
        //if (micros() - i2c_task_loop_timestamp > 10000)
        //    ESP_LOGW("i2cTask", "DS3231 read take %d uS", micros() - i2c_task_loop_timestamp);

        //FUSB302Loop();

        //i2c_task_loop_timestamp = micros();
        T6703Loop();
        //if (micros() - i2c_task_loop_timestamp > 10000)
        //    ESP_LOGW("i2cTask", "T6703 read take %d uS", micros() - i2c_task_loop_timestamp);
            
    }
}