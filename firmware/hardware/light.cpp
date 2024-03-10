#include "../h/pinsMapping.hpp"
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
    if(++count < LIGHT_MEASURE_COUNT)
    {
        return;
    }
    count = 0;
    
    raw = raw / LIGHT_MEASURE_COUNT / 27;
    //ESP_LOGI("light", "raw value: %d", raw);

    HUB75SetBrigthness(raw  > 255 ? 255: raw);
     
    raw = 0;
}