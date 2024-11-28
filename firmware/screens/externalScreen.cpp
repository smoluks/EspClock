#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "../hardware/h/hub75.hpp"
#include "../controllers/h/externalSensors.hpp"
#include "../hardware/h/systick.hpp"
#include "../hardware/h/touch.hpp"
#include "h/screenCommon.hpp"
#include "h/externalScreen.hpp"

extern void (*SingleTapHandler)();
extern void (*HoldTapHandler)();
extern MatrixPanel_I2S_DMA *dmaDisplay;

static timestamp_uS_t externalScreenShowTimestamp = 0;
static timestamp_uS_t externalScreenCloseTimestamp = 0;
static bool externalScreenIsLocked = false;
static bool externalScreenIsSingleTap = false;

static void externalScreenSingleTapHandler() { externalScreenIsSingleTap = true; }
static void externalScreenHoldTapHandler() { externalScreenIsLocked = !externalScreenIsLocked; }

void externalScreenInit()
{
    externalScreenCloseTimestamp = GetTimestamp(EXTERNAL_SCREEN_SHOW * 1000);
    externalScreenShowTimestamp = GetTimestamp(-1);
    dmaDisplay->setTextSize(1);

    SingleTapHandler = externalScreenSingleTapHandler;
    HoldTapHandler = externalScreenHoldTapHandler;
}

screen_action_t externalScreenLoop()
{
    if (externalScreenIsSingleTap || (!externalScreenIsLocked && IsTimeout(externalScreenCloseTimestamp)))
    {
        externalScreenIsLocked = false;
        externalScreenIsSingleTap = false;
        SingleTapHandler = NULL;
        HoldTapHandler = NULL;

        return SCREEN_ACTION_GO_TO_DEFAULT;
    }

    if (!IsTimeout(externalScreenShowTimestamp))
        return SCREEN_ACTION_NOTHING;

    externalScreenShowTimestamp = GetTimestamp(EXTERNAL_SCREEN_REFRESH_PERIOD * 1000);

    dmaDisplay->fillScreenRGB888(0, 0, 0);

    if (externalScreenIsLocked)
    {
        dmaDisplay->setTextColor(RED565);
        dmaDisplay->setCursor(57, 0);
        dmaDisplay->print("L");
    }

    dmaDisplay->setTextColor(WHITE565);
    dmaDisplay->setCursor(0, 0);
    dmaDisplay->print("RSSI: ");
    if(ExternalConnected && ExternalRSSIPresent)
        dmaDisplay->print(ExternalRSSI);
    else
        dmaDisplay->print("-");

    dmaDisplay->setTextColor(GREEN565);
    dmaDisplay->setCursor(4, 8);
    ExternalTemperaturePresent ? dmaDisplay->print(ExternalTemperature) : dmaDisplay->print("-");
    dmaDisplay->print(" C");

    dmaDisplay->setTextColor(BLUE565);
    dmaDisplay->setCursor(4, 16);
    ExternalHumidityPresent ? dmaDisplay->print(ExternalHumidity) : dmaDisplay->print("-");
    dmaDisplay->print(" %");

    dmaDisplay->setTextColor(YELLOW565);
    dmaDisplay->setCursor(4, 24);
    dmaDisplay->print("BAT: ");
    ExternalBatteryLevelPresent ? dmaDisplay->print(ExternalBatteryLevel) : dmaDisplay->print("-");
    dmaDisplay->print(" %");

    return SCREEN_ACTION_NOTHING;
}