#include "hardware/hub75.cpp"
#include "hardware/ds3231.cpp"
#include "hardware/bme280.cpp"
#include "hardware/fusb302.cpp"
#include "hardware/light.cpp"
#include "hardware/voltage.cpp"
#include "hardware/current.cpp"
#include "hardware/T6703.cpp"
#include "hardware/i2c.cpp"
#include "hardware/wifi.cpp"
#include "hardware/touch.cpp"
#include "hardware/DY1703.cpp"
#include "contollers/co2Controller.cpp"
#include "contollers/airSensorsController.cpp"
#include "contollers/systickController.cpp"
#include "helpers/unixTimeConverter.cpp"
#include "screens/clockScreen.cpp"
#include "screens/sensorsScreen.cpp"
#include "screens/errorsScreen.cpp"
#include "screens/effectsScreen.cpp"
#include "tasks/i2cTask.cpp"
#include "managers/settingsManager.cpp"
#include "managers/errorManager.cpp"
#include "managers/screenManager.cpp"

TaskHandle_t screenTaskHandler;
TaskHandle_t wifiTaskHandler;
TaskHandle_t i2cTaskHandler;
uint32_t firmware_loop_timestamp;

void setup()
{
  // esp_log_level_set("*", ESP_LOG_WARN);

  Serial.begin(115200);
  Serial.setDebugOutput(true);

  ESP_LOGI("firmware", "setup() running on core %d", xPortGetCoreID());
  // ESP_LOGI("firmware", "size of size_t: %d", sizeof(size_t));
  ESP_LOGI("firmware", "ESP32 chip model = %s Rev %d, has %d cores", ESP.getChipModel(), ESP.getChipRevision(), ESP.getChipCores());
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8)
  {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  ESP_LOGI("firmware", "Chip ID: 0x%X", chipId);

  loadSettings();
  lightInit();
  voltageInit();
  currentInit();
  audioInit();
  WiFiInit();

  xTaskCreatePinnedToCore(
      i2cTask,
      "i2cTask",
      2000,
      NULL,
      0,
      &i2cTaskHandler,
      1);

  HUB75Init();

  ESP_LOGI("main", "Init complete");

  firmware_loop_timestamp = millis();
}

uint32_t minimum_free_heap_size = 115000;
void loop()
{
  if (getTimePassedFrom(firmware_loop_timestamp) > 10)
    ESP_LOGW("firmware", "loop() interval %d ms", getTimePassedFrom(firmware_loop_timestamp));
  firmware_loop_timestamp = millis();

  uint32_t current_free_heap_size = esp_get_free_heap_size();
  if(current_free_heap_size < minimum_free_heap_size)
  {
    minimum_free_heap_size = current_free_heap_size;
    ESP_LOGW("firmware", "Free memory: %d bytes", minimum_free_heap_size);
  }

  lightLoop();
  voltageLoop();
  currentLoop();
  touchLoop();

  WiFiLoop();

  screenManagerLoop();

  delay(1); //Do esp32 internal stuff
}
