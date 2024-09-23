#include "h/includes.hpp"
//#include "esp_clk.h"

TaskHandle_t i2cTaskHandler;
uint32_t firmware_loop_timestamp;

void setup()
{
  //Serial.setTxBufferSize(1024);
  Serial.begin(DEBUG_SPEED);
  Serial.setDebugOutput(true);

  ESP_LOGI("firmware", "setup() running on core %d", xPortGetCoreID());
  ESP_LOGI("firmware", "CPU frequency: %d MHz, XTAL frequency: %d MHz, APB frequency: %d MHz", getCpuFrequencyMhz(), getXtalFrequencyMhz(), getApbFrequency() / 1000000);
  ESP_LOGI("firmware", "ESP32 chip model = %s Rev %d, has %d cores", ESP.getChipModel(), ESP.getChipRevision(), ESP.getChipCores());
  ESP_LOGI("firmware", "Free heap: %dB", esp_get_free_heap_size());
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8)
  {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  ESP_LOGI("firmware", "Chip ID: 0x%X", chipId);

  voltageInit();
  CurrentInit();

  PowerInit();
  ClockInit();
  CO2Init();
  
  I2CInit();
  FUSB302Init();
  xTaskCreatePinnedToCore(
      i2cTask,
      "i2cTask",
      2048,
      NULL,
      0,
      &i2cTaskHandler,
      1);

  loadSettings();

  lightInit();

  DY1703Init();

  HUB75Init();

  WiFiInit();

  ESP_LOGI("main", "Init complete");
}

uint32_t free_heap_size = 64000;
void loop()
{
  // if (getTimePassedFrom(firmware_loop_timestamp) > 10)
  //   ESP_LOGW("firmware", "loop() interval %d ms", getTimePassedFrom(firmware_loop_timestamp));
  // firmware_loop_timestamp = millis();

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
  uint32_t current_free_heap_size = esp_get_free_heap_size();
  if(current_free_heap_size < free_heap_size)
  {
     free_heap_size = current_free_heap_size;
     ESP_LOGI("firmware", "Free RAM: %d bytes", current_free_heap_size);
  }
#endif
  CheckPowerLimit();
  
  lightLoop();
  voltageLoop();
  TouchLoop();
  WiFiLoop();
  //WebServerLoop();

  screenManagerLoop();

  delay(1); // Do esp32 internal stuff
}
