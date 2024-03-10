#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "../contollers/h/systickController.hpp"
#include "../managers/h/errorManager.hpp"
#include "../hardware/h/hub75.hpp"
#include "h/screenCommon.hpp"
#include "h/errorsScreen.hpp"

extern MatrixPanel_I2S_DMA *dma_display;
uint32_t error_screen_show_timestamp = 0;

void errorScreenInit()
{
    dma_display->setTextColor(RED565);
    dma_display->setTextSize(1);

    error_screen_show_timestamp = getTimestamp(-1);
}

screen_action_t errorScreenLoop()
{
    if(!isErrors())
        return SCREEN_ACTION_GO_TO_DEFAULT;

    if (!isTimeout(error_screen_show_timestamp))
        return SCREEN_ACTION_NOTHING;
    error_screen_show_timestamp = getTimestamp(ERROR_SCREEN_REFRESH_PERIOD);
    
    dma_display->fillScreenRGB888(0, 0, 0);    


    sys_error_t* errors = getErrors();
    for(uint8_t i = 0; i < getErrorsCount(); i++)
    {   
        dma_display->setCursor((i / 4) * 16, (i % 4) * 8);

        dma_display->print('E');
        dma_display->print(errors[i]);
        dma_display->print('\n');
    }

    return SCREEN_ACTION_NOTHING;
}