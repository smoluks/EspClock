#include "driver/touch_pad.h"
#include "../h/prototheads.hpp"
#include "../h/hardware.hpp"
#include "h/touch.hpp"
#include "h/systick.hpp"

#define TOUCH_THRESH_NO_USE   (0)
#define TOUCH_THRESH_PERCENT  (80)
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)

static const char *TOUCH_TAG = "Touch";

void (*SingleTapHandler)();
void (*HoldTapHandler)();

static uint16_t previous_value;
static bool isTouched = false;
static timestamp_uS_t touchTimestamp;
static bool isUntouched = false;
static timestamp_uS_t untouchTimestamp;
static void filterFinishedCallback(uint16_t *raw_value, uint16_t *filtered_value)
{
    uint16_t current_value = filtered_value[TOUCH_PAD_NUM9];
    if(!isTouched && current_value + TOUCH_ON_HYSTERESIS < previous_value)
    {
        isTouched = true;
        touchTimestamp = GetTimestamp();
    } else if(current_value - TOUCH_OFF_HYSTERESIS > previous_value)
    {
        isUntouched = true;
        untouchTimestamp = GetTimestamp();
    }

    previous_value = current_value;
}

void TouchInit()
{
    ESP_LOGI(TOUCH_TAG, "TouchInit started");

    ESP_ERROR_CHECK(touch_pad_init());

    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    touch_pad_config(TOUCH_PAD_NUM9, 0);
    touch_pad_filter_start(10);
    touch_pad_set_trigger_mode(TOUCH_TRIGGER_MAX);
    touch_pad_set_filter_read_cb(filterFinishedCallback);
}

void TouchLoop()
{    
    if (isTouched && !isUntouched && GetTimestamp() - touchTimestamp >= TOUCH_TIME_FOR_HOLD_MS * 1000)
    {
        if (HoldTapHandler)
            HoldTapHandler();
        
        isTouched = false;
    }

    if (isTouched && isUntouched)
    {
        if (SingleTapHandler)
            SingleTapHandler();
        
        isTouched = false;
    }

    isUntouched = false;
}
