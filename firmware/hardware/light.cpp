#include "../h/hardware.hpp"
#include "../h/settings.hpp"
#include "h/hub75.hpp"
#include "h/light.hpp"

extern settings_t settings;

void lightInit()
{
    pinMode(LIGHT_ANALOG_PIN, ANALOG);

    if(!settings.light_auto)
    {
        HUB75SetBrigthness(settings.light_level);
    }
}

uint32_t raw = 0;
uint16_t count = 0;
void lightLoop()
{
    if(!settings.light_auto)
    {
        return;
    }
    
    raw += analogRead(LIGHT_ANALOG_PIN);
    if(++count < 1 << LIGHT_MEASURE_COUNT_BIT)
    {
        return;
    }
    count = 0;    
    raw = raw >> LIGHT_MEASURE_COUNT_BIT;

    //ESP_LOGI("light", "raw value: %d", raw);
    raw = raw / 27;

    HUB75SetBrigthness(raw  > 255 ? 255: raw);
     
    raw = 0;
}