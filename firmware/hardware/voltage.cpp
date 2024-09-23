#include "h/adc_LUT.hpp"
#include "../h/hardware.hpp"

static const char *VOLTAGE_TAG = "Voltage";

void voltageInit()
{
    pinMode(VOLTAGE_PIN, ANALOG);
}

static uint32_t voltage_raw = 0;
static uint8_t voltage_count = 0;
void voltageLoop()
{
    voltage_raw += analogRead(VOLTAGE_PIN);
    if(++voltage_count < 64)
    {
        return;
    }
    voltage_count = 0;
    
    voltage_raw = voltage_raw >> 6;
    ESP_LOGV(VOLTAGE_TAG, "voltage: %f V", ADC_LUT_FLOAT[voltage_raw] / 201.4034033);
   
    voltage_raw = 0;
}