#include "hardware/hub75.cpp"
#include "hardware/h/hub75.hpp"
#include "hardware/ds3231.cpp"
#include "hardware/h/ds3231.hpp"
#include "hardware/bme280.cpp"
#include "hardware/h/bme280.hpp"
#include "hardware/FUSB302.cpp"
#include "hardware/light.cpp"
#include "hardware/T6703.cpp"
#include "hardware/i2c.cpp"
#include "hardware/wifi.cpp"
#include "helpers/unixTimeConverter.cpp"
#include "helpers/h/unixTimeConverter.hpp"
#include "screens/clock.hpp"
#include "screens/clock.cpp"
#include "screens/temperature.cpp"

void setup()
{
  esp_log_level_set("*", ESP_LOG_INFO);

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  delay(500);
  
  Hub75Init();

  FUSB302Init();

  I2CInit();
  
  DS3231Init();

  BME280Init();

  T6703Init();
  
  //Hub75TestScreen();

  screen_clock_init();
  
  ESP_LOGI("main", "Init complete");
}

void loop()
{
  //ESP_LOGI("main", "Loop");

  FUSB302_loop();

  screen_clock_process();

  light_loop();

  T6703Loop();
  
  WIFIProcess();
}
