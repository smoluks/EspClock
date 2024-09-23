#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "../hardware/h/hub75.hpp"
#include "../controllers/h/airInternal.hpp"
#include "../controllers/h/co2.hpp"
#include "../controllers/h/systick.hpp"
#include "../hardware/h/touch.hpp"
#include "h/screenCommon.hpp"
#include "h/sensorsScreen.hpp"

extern void (*SingleTapHandler)();
extern void (*HoldTapHandler)();

static timestamp_uS_t sensor_screen_show_timestamp = 0;
static timestamp_uS_t sensor_screen_close_timestamp = 0;
static bool isLocked = false;
static bool sensorsScreenIsSingleTap = false;
extern MatrixPanel_I2S_DMA *dma_display;

void sensorsScreenSingleTapHandler() { sensorsScreenIsSingleTap = true; }
void sensorsScreenHoldTapHandler() { isLocked = !isLocked; }

void sensorsScreenInit()
{
    sensor_screen_close_timestamp = GetTimestamp(SENSORS_SCREEN_SHOW_TIME * 1000);
    sensor_screen_show_timestamp = GetTimestamp(-1);
    dma_display->setTextSize(1);

    SingleTapHandler = sensorsScreenSingleTapHandler;
    HoldTapHandler = sensorsScreenHoldTapHandler;
}

screen_action_t sensorsScreenLoop()
{
    if (sensorsScreenIsSingleTap || (!isLocked && IsTimeout(sensor_screen_close_timestamp)))
    {
        isLocked = false;
        sensorsScreenIsSingleTap = false;
        SingleTapHandler = NULL;
        HoldTapHandler = NULL;

        return SCREEN_ACTION_GO_TO_DEFAULT;
    }        

    if (!IsTimeout(sensor_screen_show_timestamp))
        return SCREEN_ACTION_NOTHING;

    sensor_screen_show_timestamp = GetTimestamp(SENSORS_SCREEN_REFRESH_PERIOD * 1000);

    dma_display->fillScreenRGB888(0, 0, 0);

    if(isLocked)
    {
        dma_display->setTextColor(RED565);
        dma_display->setCursor(57, 0);
        dma_display->print("L");
    }

    dma_display->setTextColor(GREEN565);
    dma_display->setCursor(1, 0);
    IsInternalTemperaturePresent() ? dma_display->print(GetInternalTemperature()) : dma_display->print("-");
    dma_display->print(" C");

    dma_display->setTextColor(MAGENTA565);
    dma_display->setCursor(5, 8);
    IsInternalPressurePresent() ? dma_display->print(GetInternalPressure(), 0) : dma_display->print("-");
    dma_display->print(" hPa");

    dma_display->setTextColor(BLUE565);
    dma_display->setCursor(1, 16);
    IsInternalHumidityPresent() ? dma_display->print(GetInternalHumidity()) : dma_display->print("-");
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