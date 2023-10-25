#include "hardware/hub75.hpp"
#include "hardware/hub75.cpp"
#include "hardware/ds3231.cpp"
#include "hardware/FUSB302.cpp"
#include "hardware/light.cpp"
#include "screens/clock.hpp"
#include "screens/clock.cpp"
#include "screens/clock.hpp"

void setup()
{
  esp_log_level_set("*", ESP_LOG_INFO);

  Serial.begin(115200);
  Serial.setDebugOutput(true);

  hub75_init();

  ds3231Init();

  FUSB302_init();

  screen_clock_init();
  
  ESP_LOGI("main", "Init complete");
}

void loop()
{
  //ESP_LOGI("main", "Loop");

  FUSB302_loop();

  screen_clock_process();

  light_loop();
}
