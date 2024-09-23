#include "../h/prototheads.hpp"
#include "h/touch.hpp"
#include "../h/hardware.hpp"
#include "../controllers/h/systick.hpp"

static const char *TOUCH_TAG = "Touch";

void processTouch();
void processUntouch();

void (*SingleTapHandler)();
void (*HoldTapHandler)();

static struct pt touch_pt;
static bool is_touched = false;
static timestamp_uS_t touch_timestamp;
static timestamp_uS_t touch_process_timestamp = 0;
static uint16_t touch_previous_value = 0;
static bool hold_touched_evt_readed = false;
uint8_t TouchLoop()
{
    PT_BEGIN(&touch_pt);

    PT_WAIT_UNTIL(&touch_pt, IsTimeout(touch_process_timestamp));

    touch_process_timestamp = GetTimestamp(TOUCH_PROCESS_CYCLE_MS * 1000);

    //-----check value-----
    uint16_t value = touchRead(TOUCH_PIN);
    // if(value != touch_previous_value)
    //     ESP_LOGV(TOUCH_TAG, "touch raw value: %d", value);

    int16_t diff = value - touch_previous_value;
    touch_previous_value = value;
    if (diff <= -TOUCH_ON_HYSTERESIS)
    {
        processTouch();
    }
    if (diff >= TOUCH_ON_HYSTERESIS)
    {
        processUntouch();
    }

    //-----check hold condition-----
    if (is_touched && !hold_touched_evt_readed && GetTimestamp() - touch_timestamp >= TOUCH_TIME_FOR_HOLD_MS * 1000)
    {
        ESP_LOGI(TOUCH_TAG, "hold touch evt, handler: %p", HoldTapHandler);
        if (HoldTapHandler)
            HoldTapHandler();

        hold_touched_evt_readed = true;
    }

    PT_END(&touch_pt);
}

inline void processTouch()
{
    ESP_LOGI(TOUCH_TAG, "touched");

    if(is_touched)
        return;

    is_touched = true;
    touch_timestamp = GetTimestamp();

    ESP_LOGI(TOUCH_TAG, "touch evt, handler: %p", SingleTapHandler);
    if (SingleTapHandler)
        SingleTapHandler();
}

inline void processUntouch()
{
    ESP_LOGI(TOUCH_TAG, "untouched");

    is_touched = false;
    hold_touched_evt_readed = false;
}