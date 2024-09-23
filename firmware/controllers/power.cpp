#include "../hardware/h/FUSB302.hpp"
#include "../hardware/h/current.hpp"
#include "../hardware/h/HUB75.hpp"

static const char *POWER_CONTROLLER_TAG = "PowerController";

extern void (*PowerChangedHandler)(status_power_e status, uint16_t voltage, uint16_t current);

/// voltage and current in mV, mA
static uint16_t current_limit = 500;
static void powerChangedHandler(status_power_e status, uint16_t voltage, uint16_t current)
{
    if(current > current_limit)
    {
        HUB75ResetBrigthnessLimit();
    }

    current_limit = current;
}

void PowerInit()
{
    PowerChangedHandler = powerChangedHandler;
}

void CheckPowerLimit()
{
    int16_t current = CurrentLoop();
    if(current == -1)
    {
        return;
    }

    if(current > current_limit)
    {
        HUB75DecreaseBrigthnessLimit();
        ESP_LOGE(POWER_CONTROLLER_TAG, "Current overload: permitted %d mA, real %d mA", current_limit, current);
    }
}