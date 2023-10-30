#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "../hardware/h/hub75.hpp"

unsigned long temp_show_timestamp = 0;

void screen_temperature_init()
{  
    dma_display->setTextSize(1);
}

void screen_temperature_process()
{
    if (millis() - temp_show_timestamp < 1000)
    {
        return;
    }
    temp_show_timestamp = millis();

    dma_display->fillScreenRGB888(0, 0, 0);

    dma_display->setTextColor(GREEN565);
    dma_display-> setCursor(0, 0);
    dma_display->print(BME280GetTemperature());
    dma_display->print(" C"); 

    dma_display->setTextColor(MAGENTA565);
    dma_display-> setCursor(0, 8);
    dma_display->print(BME280GetPressure());
    dma_display->print("hPa"); 

    dma_display->setTextColor(BLUE565);
    dma_display-> setCursor(0, 16);
    dma_display->print(BME280GetHumidity());
    dma_display->print(" %"); 

    if(T6703IsPresent())
    {
        dma_display->setTextColor(WHITE565);
        dma_display-> setCursor(0, 24);
        dma_display->print(T6703GetCO2());
        dma_display->print(" PPM"); 
    }
}