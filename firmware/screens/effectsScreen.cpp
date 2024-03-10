#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <ESP32-VirtualMatrixPanel-I2S-DMA.h>
#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <FastLED.h>
#include "../hardware/h/hub75.hpp"
#include "h/screenCommon.hpp"

#include "testPatterns/Effects.h"
Effects effects;

#include "testPatterns/Drawable.h"
#include "testPatterns/Playlist.h"
// #include "Geometry.h"

#include "testPatterns/Patterns.h"
Patterns patterns;

void listPatterns();

extern MatrixPanel_I2S_DMA *dma_display;

unsigned long fps = 0, fps_timer;                // fps (this is NOT a matrix refresh rate!)
unsigned int default_fps = 30, pattern_fps = 30; // default fps limit (this is not a matrix refresh counter!)
unsigned long ms_animation_max_duration = 20000; // 20 seconds
unsigned long last_frame = 0, ms_previous = 0;

static const char* log_tag = "effects";

void effectsScreenInit()
{ 
  VirtualMatrixPanel* panel = new VirtualMatrixPanel(*dma_display, 1, 1, PANEL_RES_X, PANEL_RES_Y);
  panel->drawDisplayTest();
  ///panel->drawChar(0, 0, 'A', 0x07E0, 0x0000, 1);
  
  // setup the effects generator
  effects.Setup();

  //delay(500);
  //Serial.println("Effects being loaded: ");
  //listPatterns();

  patterns.moveRandom(1); // start from a random pattern

  //Serial.print("Starting with pattern: ");
  //Serial.println(patterns.getCurrentPatternName());

  patterns.start();

  ms_previous = millis();
  fps_timer = millis();
}

screen_action_t effectsScreenLoop()
{
  // menu.run(mainMenuItems, mainMenuItemCount);

  if ((millis() - ms_previous) > ms_animation_max_duration)
  {
    patterns.stop();
    patterns.moveRandom(1);
    // patterns.move(1);
    patterns.start();

    //Serial.print("Changing pattern to:  ");
    //Serial.println(patterns.getCurrentPatternName());

    ms_previous = millis();

    // Select a random palette as well
    // effects.RandomPalette();
  }

  if (1000 / pattern_fps + last_frame < millis())
  {
    last_frame = millis();
    pattern_fps = patterns.drawFrame();
    if (!pattern_fps)
      pattern_fps = default_fps;

    ++fps;
  }

  if (fps_timer + 1000 < millis())
  {
    //Serial.printf_P(PSTR("Effect fps: %ld\n"), fps);
    fps_timer = millis();
    fps = 0;
  }
}

void listPatterns()
{
  patterns.listPatterns();
}