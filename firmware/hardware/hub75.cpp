#include <ESP32-VirtualMatrixPanel-I2S-DMA.h>
//#include <Fonts/FreeMonoBoldOblique12pt7b.h>
//#include <Fonts/FreeSerif9pt7b.h>
#include <FastLED.h>
#include "h/hub75.hpp"
#include "../h/hardware.hpp"
#include "../h/settings.hpp"

static const char *HUB75_TAG = "HUB75";

extern settings_t settings;
MatrixPanel_I2S_DMA *dma_display = nullptr;

// Module configuration
static HUB75_I2S_CFG::i2s_pins _pins = {R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};
static HUB75_I2S_CFG mxconfig(
    PANEL_RES_X, // module width
    PANEL_RES_Y, // module height
    PANEL_CHAIN, // Chain length
    _pins,
    HUB75_I2S_CFG::SHIFTREG,         // driver chip,
    false,                           // double buffer
    HUB75_I2S_CFG::HZ_20M,           // i2s speed
    DEFAULT_LAT_BLANKING,            // How many clock cycles to blank OE before/after LAT signal change, default is 2 clocks
    false,                           // clock phase
    60,                              // _min_refresh_rate
    PIXEL_COLOR_DEPTH_BITS_DEFAULT); // _pixel_color_depth_bits
static uint8_t maximum_brigthness = 255;
void HUB75Init()
{
  ESP_LOGI(HUB75_TAG, "HUB75 init started");

  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();

  HUB75SetBrigthness(50);

  dma_display->fillScreenRGB888(128, 0, 0);
  delay(500);
  dma_display->fillScreenRGB888(0, 0, 128);
  delay(500);
  dma_display->fillScreenRGB888(0, 128, 0);
  delay(500);
  dma_display->fillScreenRGB888(255, 255, 255);
  delay(500);
  dma_display->fillScreenRGB888(0, 0, 0);

  if(!settings.light_auto)
    HUB75SetBrigthness(settings.light_level);

  ESP_LOGI(HUB75_TAG, "HUB75 init completed");
}

// set brightness 0 - 255
static uint8_t current_brigthness;
void HUB75SetBrigthness(uint8_t brightness)
{
  ESP_LOGV(HUB75_TAG, "Set brightness = %d, maximum %d, minimum %d", brightness, maximum_brigthness, MIN_BRIGHTNESS);

  if (maximum_brigthness && brightness > maximum_brigthness)
    brightness = maximum_brigthness;

  if (brightness < MIN_BRIGHTNESS)
  {
    brightness = MIN_BRIGHTNESS;
  }

  dma_display->setBrightness(brightness);
  current_brigthness = brightness;
}

void HUB75DecreaseBrigthnessLimit()
{
  maximum_brigthness = current_brigthness - 2;
  dma_display->setBrightness(maximum_brigthness);
}

void HUB75ResetBrigthnessLimit()
{
  maximum_brigthness = 255;
  if(!settings.light_auto)
    HUB75SetBrigthness(settings.light_level);
}