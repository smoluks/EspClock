#include "h/adc_LUT.hpp"
#include "h/fusb302.hpp"
#include "../h/hardware.hpp"

void CurrentInit()
{
    pinMode(CURRENT_PIN, ANALOG);
    analogSetPinAttenuation(CURRENT_PIN, ADC_0db);
}

uint32_t current_raw = 0;
uint8_t current_count = 0;
void CurrentLoop()
{
    current_raw += analogRead(CURRENT_PIN);
    if(++current_count < 64)
    {
        return;
    }
    current_count = 0;
    
    current_raw = current_raw / 64;
    float value = ADC_LUT_FLOAT[current_raw] / 1.963134918;
    ESP_LOGV("Current", "current: %f mA", value);
    
    if(value > GetPermittedUSBCurrent())
    {
        ESP_LOGE("Current", "Current overload: permitted %d mA, real %f mA", GetPermittedUSBCurrent(), value);
    }

    current_raw = 0;
}