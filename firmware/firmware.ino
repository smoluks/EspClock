#include "hardware/hub75.hpp"
#include "hardware/hub75.cpp"
#include "hardware/ds3231.cpp"
#include "hardware/FUSB302.cpp"
#include "hardware/bme280.cpp"
#include "hardware/light.cpp"
#include "hardware/T6703.cpp"
#include "screens/clock.hpp"
#include "screens/clock.cpp"
#include "screens/temperature.cpp"

void setup()
{
  esp_log_level_set("*", ESP_LOG_INFO);

  Serial.begin(115200);
  Serial.setDebugOutput(true);

  Hub75Init();

  DS3231Init();

  FUSB302Init();

  BME280Init();

  T6703Init();
  
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
