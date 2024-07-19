#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "../hardware/h/hub75.hpp"
#include "../hardware/h/touch.hpp"
#include "../controllers/h/clockController.hpp"
#include "../controllers/h/co2Controller.hpp"
#include "h/screenCommon.hpp"
#include "h/clockScreen.hpp"

void moveCursor();
const char *GetDayOfWeekUserName(uint8_t day);

extern MatrixPanel_I2S_DMA *dma_display;

static uint8_t current_x;
static uint8_t current_y;
static int8_t speed_x;
static int8_t speed_y;

void clockScreenInit()
{
    current_x = random(5);
    current_y = random(19);

    speed_x = random(2) == 1 ? 1 : -1;
    speed_y = random(2) == 1 ? 1 : -1;
}

uint8_t oldMinute = 255;
uint32_t clock_page_show_timestamp = 0;
screen_action_t clockScreenLoop()
{
    if (isTouched())
    {
        return SCREEN_ACTION_GO_TO_NEXT;
    }

    if (millis() - clock_page_show_timestamp < CLOCK_MOVE_PERIOD)
        return SCREEN_ACTION_NOTHING;
    clock_page_show_timestamp = millis();

    dma_display->fillScreenRGB888(0, 0, 0);

    if (!IsTimePresent())
    {
        dma_display->setTextColor(GREEN565);
        dma_display->setTextSize(1);
        dma_display->print("Loading...");
    }
    else
    {
        DateTime currentTime = GetCurrentTime();
        const char time[6] = {
            0x30 + currentTime.hour / 10,
            0x30 + currentTime.hour % 10,
            ':',
            0x30 + currentTime.minute / 10,
            0x30 + currentTime.minute % 10,
            0};

        //-----Time-----
        moveCursor();
        dma_display->setTextColor(GREEN565);
        dma_display->setTextSize(2);
        dma_display->print(time);

        //-----Week day-----
        dma_display->setCursor(0, 25);
        dma_display->setTextColor(BLUE565);
        dma_display->setTextSize(1);
        dma_display->print(GetDayOfWeekUserName(currentTime.dayOfWeek));

        //-----Date-----
        const char date[6] = {
            0x30 + currentTime.date / 10,
            0x30 + currentTime.date % 10,
            '.',
            0x30 + currentTime.month / 10,
            0x30 + currentTime.month % 10,
            0};

        dma_display->setCursor(34, 25);
        dma_display->setTextColor(RED565);
        dma_display->setTextSize(1);
        dma_display->print(date);
    }

    //-----CO2 Warning-----
    if (IsCO2Present())
    {
        if (GetCO2Value() > 1000)
            dma_display->drawPixel(63, 0, RED565);
        else if (GetCO2Value() > 750)
            dma_display->drawPixel(63, 0, ORANGE565);
    }

    return SCREEN_ACTION_NOTHING;
}

const char *GetDayOfWeekUserName(uint8_t day)
{
    switch (day)
    {
    case 1:
        return "SUN";
    case 2:
        return "MON";
    case 3:
        return "TUE";
    case 4:
        return "WED";
    case 5:
        return "THU";
    case 6:
        return "FRI";
    case 7:
        return "SAT";
    default:
        return "";
    }
}

void moveCursor()
{
    dma_display->setCursor(current_x, current_y);

    if (current_x + speed_x > 4 || current_x + speed_x < 0)
    {
        speed_x = -speed_x;
    }
    current_x = current_x + speed_x;

    if (current_y + speed_y > 18 || current_y + speed_y < 0)
    {
        speed_y = -speed_y;
    }
    current_y = current_y + speed_y;
}