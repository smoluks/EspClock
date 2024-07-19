#include "../hardware/h/i2c.hpp"
#include "../hardware/h/ds3231.hpp"
#include "../hardware/h/bme280.hpp"
#include "../hardware/h/fusb302.hpp"
#include "../hardware/h/T6703.hpp"

/// @brief Task for all i2c bus work
void i2cTask(void *parameter)
{
    ESP_LOGI("i2cTask", "i2cTask running on core %d", xPortGetCoreID());

    DS3231Init();
    FUSB302Loop();

    BME280Init();
    FUSB302Loop();

    T6703Init();
    FUSB302Loop();
    
    for (;;)
    {
        BME280Loop();
        FUSB302Loop();

        DS3231Loop();
        FUSB302Loop();

        T6703Loop();
        FUSB302Loop();
    }
}