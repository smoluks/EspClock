#include "h/adc_LUT.hpp"
#include "h/current.hpp"
#include "../h/hardware.hpp"

static const char *CURRENT_TAG = "Current";

void CurrentInit()
{
    pinMode(CURRENT_PIN, ANALOG);
    analogSetPinAttenuation(CURRENT_PIN, ADC_0db);
}

static uint32_t current_raw = 0;
static uint8_t current_count = 0;
int16_t CurrentLoop()
{
    current_raw += analogRead(CURRENT_PIN);
    if(++current_count != 64)
    {
        return -1;
    }
    current_count = 0;    
    current_raw = current_raw >> 6;

    float value = ADC_LUT_FLOAT[current_raw] * ADC_TO_mA_COEF;
    ESP_LOGV(CURRENT_TAG, "current: %f mA", value);  

    current_raw = 0;
    return value;
}
