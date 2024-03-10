#include "h/touch.hpp"
#include "../h/pinsMapping.hpp"

void processTouchValue(uint16_t value);

uint32_t touch_raw_value = 0;
uint8_t touch_count = 0;
void touchLoop()
{
    touch_raw_value += touchRead(TOUCH_PIN);    
    if(++touch_count < 64)
    {
        return;
    }
    touch_count = 0;
    
    touch_raw_value = touch_raw_value >> 6;
    ESP_LOGV("touch", "touch raw value: %d", touch_raw_value);

    processTouchValue(touch_raw_value);

    touch_raw_value = 0;
}

bool touch_old_value = false;
int touch_filter_count = 0;
bool is_touched_flag = false;
uint16_t max_value = 0;
void processTouchValue(uint16_t value)
{
    if(value > max_value)
    {
        max_value = value;
    }

    bool currentValue = max_value - value > TOUCH_ON_HYSTERESIS;    
    if(currentValue == touch_old_value)
    {
        if(touch_filter_count < TOUCH_CYCLE_COUNT)
        {
            touch_filter_count++;
        } else if (touch_filter_count == TOUCH_CYCLE_COUNT)
        {
            if(currentValue)
            {
                is_touched_flag = true;
            }
        }        
    } else {
        touch_old_value = currentValue;
        touch_filter_count = 0;
    }
}

inline bool isTouched()
{
    if(is_touched_flag)
    {
        is_touched_flag = false;
        return true;
    } else {
        return false;
    }
}
