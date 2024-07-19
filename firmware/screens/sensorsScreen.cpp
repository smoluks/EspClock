#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "../hardware/h/hub75.hpp"
#include "../controllers/h/envSensorsController.hpp"
#include "../controllers/h/co2Controller.hpp"
#include "../controllers/h/systickController.hpp"
#include "../hardware/h/touch.hpp"
#include "h/screenCommon.hpp"
#include "h/sensorsScreen.hpp"

uint32_t sensor_screen_show_timestamp = 0;
uint32_t sensor_screen_close_timestamp = 0;

extern MatrixPanel_I2S_DMA *dma_display;

void sensorsScreenInit()
{
    sensor_screen_close_timestamp = GetTimestamp(SENSORS_SCREEN_SHOW_TIME);
    sensor_screen_show_timestamp = GetTimestamp(-1);
    dma_display->setTextSize(1);
}

screen_action_t sensorsScreenLoop()
{
    if (IsTimeout(sensor_screen_close_timestamp))
        return SCREEN_ACTION_GO_TO_DEFAULT;

    if (!IsTimeout(sensor_screen_show_timestamp))
        return SCREEN_ACTION_NOTHING;
    sensor_screen_show_timestamp = GetTimestamp(SENSORS_SCREEN_REFRESH_PERIOD);

    dma_display->fillScreenRGB888(0, 0, 0);

    dma_display->setTextColor(GREEN565);
    dma_display->setCursor(1, 0);
    IsTemperaturePresent() ? dma_display->print(GetTemperature()) : dma_display->print("-");
    dma_display->print(" C");

    dma_display->setTextColor(MAGENTA565);
    dma_display->setCursor(5, 8);
    IsPressurePresent() ? dma_display->print(GetPressure(), 0) : dma_display->print("-");
    dma_display->print(" hPa");

    dma_display->setTextColor(BLUE565);
    dma_display->setCursor(1, 16);
    IsHumidityPresent() ? dma_display->print(GetHumidity()) : dma_display->print("-");
    dma_display->print(" %");

    if (IsCO2Present())
    {
        dma_display->setTextColor(WHITE565);
        dma_display->setCursor(5, 24);
        dma_display->print(GetCO2Value());
        dma_display->print(" PPM");
    }

    return SCREEN_ACTION_NOTHING;
}