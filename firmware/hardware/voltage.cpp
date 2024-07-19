#include "h/adc_LUT.hpp"
#include "../h/hardware.hpp"

void voltageInit()
{
    pinMode(VOLTAGE_PIN, ANALOG);
}

uint32_t voltage_raw = 0;
uint8_t voltage_count = 0;
void voltageLoop()
{
    voltage_raw += analogRead(VOLTAGE_PIN);
    if(++voltage_count < 64)
    {
        return;
    }
    voltage_count = 0;
    
    voltage_raw = voltage_raw >> 6;
    ESP_LOGV("Voltage", "voltage: %f V", ADC_LUT_FLOAT[voltage_raw] / 201.4034033);
   
    voltage_raw = 0;
}