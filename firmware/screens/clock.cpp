#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "../hardware/h/hub75.hpp"
#include "../hardware/h/ds3231.hpp"

void moveCursor();

extern MatrixPanel_I2S_DMA* dma_display;

static uint8_t current_x;
static uint8_t current_y;
static int8_t speed_x;
static int8_t speed_y;

void screen_clock_init()
{  
    ESP_LOGI("clock", "Clock init started");  

    dma_display->setTextColor(GREEN565);
    dma_display->setTextSize(2);

    current_x = random(5);
    current_y = random(19);

    speed_x = random(2) == 1 ? 1 : -1;
    speed_y = random(2) == 1 ? 1 : -1;

    ESP_LOGI("clock", "Clock init completed");
}

uint8_t oldMinute = 255;
unsigned long ms_previous = 0;
void screen_clock_process()
{
    if(millis() - ms_previous < 1000)
    {
        return;
    }    
    ms_previous = millis();

    DS3231RefreshTime();
    
    moveCursor();

    dma_display->fillScreenRGB888(0, 0, 0);
    
    dma_display->print(DS3231GetHour() / 10);
    dma_display->print(DS3231GetHour() % 10);
    dma_display->print(':');
    dma_display->print(DS3231GetMinute() / 10);
    dma_display->print(DS3231GetMinute() % 10);
}

void moveCursor()
{
    dma_display-> setCursor(current_x, current_y);

    if(current_x + speed_x > 4 || current_x + speed_x < 0)
    {
        speed_x = -speed_x;
    }
    current_x = current_x + speed_x;

    if(current_y + speed_y > 18 || current_y + speed_y < 0)
    {
        speed_y = -speed_y;
    }
    current_y = current_y + speed_y;
}