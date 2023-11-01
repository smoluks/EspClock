#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <ESP32-VirtualMatrixPanel-I2S-DMA.h>
#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <FastLED.h>
#include "h/hub75.hpp"

// Module configuration
HUB75_I2S_CFG::i2s_pins _pins = {R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};
HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,              // module width
    PANEL_RES_Y,              // module height
    PANEL_CHAIN,              // Chain length
    _pins,
    HUB75_I2S_CFG::SHIFTREG,  // driver chip,
    false,                    // double buffer
    HUB75_I2S_CFG::HZ_20M,    // i2s speed
    DEFAULT_LAT_BLANKING,     // How many clock cycles to blank OE before/after LAT signal change, default is 2 clocks
    false,                    // clock phase
    60,                       // _min_refresh_rate
    PIXEL_COLOR_DEPTH_BITS_DEFAULT);    // _pixel_color_depth_bits

MatrixPanel_I2S_DMA* dma_display = nullptr;

void Hub75Init()
{
  /************** DISPLAY **************/
  ESP_LOGI("hub75", "HUB75 init started");

  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness(3);
  dma_display->setTextColor(GREEN565);
  dma_display->setTextSize(1);
  dma_display->setCursor(0, 0);
  dma_display->print("Loading"); 

  ESP_LOGI("hub75", "HUB75 init completed");
}

void Hub75MoveLoadingBar()
{
  dma_display->print("."); 
}

void Hub75TestScreen()
{
  dma_display->fillScreenRGB888(128, 0, 0);
  delay(500);
  dma_display->fillScreenRGB888(0, 0, 128);
  delay(500);
  dma_display->fillScreenRGB888(0, 128, 0);
  delay(500);
  dma_display->fillScreenRGB888(255, 255, 255);
  delay(500);
  dma_display->fillScreenRGB888(0, 0, 0);
}

//set brightness 0 - 255
void Hub75SetBrigthness(uint8_t brightness)
{
  if(brightness < MIN_BRIGHTNESS)
  {
    brightness = MIN_BRIGHTNESS;
  }
  
  dma_display->setBrightness(brightness);
}
