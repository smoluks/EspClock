#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "../hardware/h/hub75.hpp"
#include "../controllers/h/airInternal.hpp"
#include "../controllers/h/co2.hpp"
#include "../hardware/h/systick.hpp"
#include "../hardware/h/touch.hpp"
#include "h/screenCommon.hpp"
#include "h/sensorsScreen.hpp"

extern void (*SingleTapHandler)();
extern void (*HoldTapHandler)();

static timestamp_uS_t sensorsScreenShowTimestamp = 0;
static timestamp_uS_t sensorsScreenCloseTimestamp = 0;
static bool sensorsScreenIsLocked = false;
static bool sensorsScreenIsSingleTap = false;
extern MatrixPanel_I2S_DMA *dmaDisplay;

void sensorsScreenSingleTapHandler() { sensorsScreenIsSingleTap = true; }
void sensorsScreenHoldTapHandler() { sensorsScreenIsLocked = !sensorsScreenIsLocked; }

void sensorsScreenInit()
{
    sensorsScreenCloseTimestamp = GetTimestamp(SENSORS_SCREEN_SHOW_TIME * 1000);
    sensorsScreenShowTimestamp = GetTimestamp(-1);
    dmaDisplay->setTextSize(1);

    SingleTapHandler = sensorsScreenSingleTapHandler;
    HoldTapHandler = sensorsScreenHoldTapHandler;
}

screen_action_t sensorsScreenLoop()
{
    if (sensorsScreenIsSingleTap)
    {
        sensorsScreenIsLocked = false;
        sensorsScreenIsSingleTap = false;
        SingleTapHandler = NULL;
        HoldTapHandler = NULL;

        return SCREEN_ACTION_GO_TO_NEXT;
    }        

    if (!sensorsScreenIsLocked && IsTimeout(sensorsScreenCloseTimestamp))
    {
        sensorsScreenIsLocked = false;
        sensorsScreenIsSingleTap = false;
        SingleTapHandler = NULL;
        HoldTapHandler = NULL;

        return SCREEN_ACTION_GO_TO_DEFAULT;
    }        

    if (!IsTimeout(sensorsScreenShowTimestamp))
        return SCREEN_ACTION_NOTHING;

    sensorsScreenShowTimestamp = GetTimestamp(SENSORS_SCREEN_REFRESH_PERIOD * 1000);

    dmaDisplay->fillScreenRGB888(0, 0, 0);

    if(sensorsScreenIsLocked)
    {
        dmaDisplay->setTextColor(RED565);
        dmaDisplay->setCursor(57, 0);
        dmaDisplay->print("L");
    }

    dmaDisplay->setTextColor(GREEN565);
    dmaDisplay->setCursor(1, 0);
    IsInternalTemperaturePresent() ? dmaDisplay->print(GetInternalTemperature()) : dmaDisplay->print("-");
    dmaDisplay->print(" C");

    dmaDisplay->setTextColor(MAGENTA565);
    dmaDisplay->setCursor(5, 8);
    IsInternalPressurePresent() ? dmaDisplay->print(GetInternalPressure(), 0) : dmaDisplay->print("-");
    dmaDisplay->print(" hPa");

    dmaDisplay->setTextColor(BLUE565);
    dmaDisplay->setCursor(1, 16);
    IsInternalHumidityPresent() ? dmaDisplay->print(GetInternalHumidity()) : dmaDisplay->print("-");
    dmaDisplay->print(" %");

    if (IsCO2Present())
    {
        dmaDisplay->setTextColor(WHITE565);
        dmaDisplay->setCursor(5, 24);
        dmaDisplay->print(GetCO2Value());
        dmaDisplay->print(" PPM");
    }

    return SCREEN_ACTION_NOTHING;
}