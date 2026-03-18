#include "h/includes.hpp"
// #include "esp_clk.h"

static const char *FIRMWARE_TAG = "firmware";

void setup()
{
  Serial.begin(DEBUG_SPEED);
  Serial.setDebugOutput(true);

  I2CTaskInit();

  ESP_LOGI(FIRMWARE_TAG, "setup() running on core %d", xPortGetCoreID());
  ESP_LOGI(FIRMWARE_TAG, "CPU frequency: %d MHz, XTAL frequency: %d MHz, APB frequency: %d MHz", getCpuFrequencyMhz(), getXtalFrequencyMhz(), getApbFrequency() / 1000000);
  ESP_LOGI(FIRMWARE_TAG, "ESP32 chip model = %s Rev %d, has %d cores", ESP.getChipModel(), ESP.getChipRevision(), ESP.getChipCores());
  ESP_LOGI(FIRMWARE_TAG, "Free heap: %dB", esp_get_free_heap_size());
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8)
  {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  ESP_LOGI(FIRMWARE_TAG, "Chip ID: 0x%X", chipId);

  HUB75Init();

  VoltageInit();
  CurrentInit();
  LightInit();

  // DY1703Init();
  //  LoadSettings();

  TouchInit();

  WifiInit();

  // ExternalSensorsInit();

  ESP_LOGI(FIRMWARE_TAG, "setup() complete");
}

timestamp_uS_t firmwareLoopTimestamp;
uint32_t freeHeapSizeWarning = 10000;
void loop()
{
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO

  if (!firmwareLoopTimestamp)
  {
    ESP_LOGI(FIRMWARE_TAG, "loop() running on core %d", xPortGetCoreID());
  }
  else if (GetTimestamp() - firmwareLoopTimestamp > 10000)
  {
    ESP_LOGW(FIRMWARE_TAG, "loop() interval %d us", GetTimestamp() - firmwareLoopTimestamp);
  }
  firmwareLoopTimestamp = GetTimestamp();

  uint32_t current_free_heap_size = esp_get_free_heap_size();
  if (current_free_heap_size < freeHeapSizeWarning)
  {
    freeHeapSizeWarning = current_free_heap_size;
    ESP_LOGE(FIRMWARE_TAG, "Free heap: %d bytes", current_free_heap_size);
  }
#endif

  CheckPowerLimit();

  lightLoop();
  VoltageLoop();
  TouchLoop();
  ClockLoop();

  // ExternalSensorsLoop();

  screenManagerLoop();

  delay(1); // Do esp32 internal stuff
}
