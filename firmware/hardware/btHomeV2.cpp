// protocol see https://bthome.io/
// devices see https://pvvx.github.io/

#include <esp_gatt_defs.h>
#include "../h/settings.hpp"
#include "h/systick.hpp"
#include "h/bleClient.hpp"
#include "h/btHomeV2.hpp"

#define BTHOME_CONNECTION_PERIOD_US 60 * 1000 * 1000 // BLE stack connection timeout 30S, so it's practical to use greater value
#define BTHOME_RSSI_READ_PERIOD_US 60 * 1000 * 1000
#define AMBIENT_SERVICE_ID 0x181a
#define AMBIENT_TEMPERATURE_CHAR_ID 0x2a6e
#define AMBIENT_TEMPERATURE_HUMIDITY_CHAR_ID 0x2a6f
#define BATTERY_SERVICE_ID 0x180f
#define BATTERY_CHAR_ID 0x2a19

static const char *BTHOMEV2_TAG = "BTHomeV2";

void connectCb();
void disconnectCb();
void readCb(uint16_t handle, uint16_t length, uint8_t *value);
void rssiCb(int8_t rssi);

static float_callback _temperatureCallback;
static float_callback _humidityCallback;
static int8_callback _rssiCallback;
static int8_callback _batteryCallback;
static bool_callback _connectCallback;

extern settings_t settings;
static esp_gattc_service_elem_t btHomeAmbientService, btHomeBatteryService;
static esp_gattc_char_elem_t btHomeTemperatureChar, btHomeHumidityChar, btHomeBatteryChar;
static timestamp_uS_t BTHomeNextLoopTimestamp;
void BTHomeInit(float_callback temperatureCallback,
                float_callback humidityCallback,
                int8_callback rssiCallback,
                int8_callback batteryCallback,
                bool_callback connectCallback)
{
  ESP_LOGI(BTHOMEV2_TAG, "init started");

  _temperatureCallback = temperatureCallback;
  _humidityCallback = humidityCallback;
  _rssiCallback = rssiCallback;
  _batteryCallback = batteryCallback;
  _connectCallback = connectCallback;

  btHomeTemperatureChar.char_handle = 0;
  btHomeHumidityChar.char_handle = 0;
  btHomeBatteryChar.char_handle = 0;

  if (BLEClientInit())
  {
    BTHomeNextLoopTimestamp = GetTimestamp();
    ESP_LOGI(BTHOMEV2_TAG, "init finished");
  }
  else
  {
    BTHomeNextLoopTimestamp = 0;
    ESP_LOGE(BTHOMEV2_TAG, "init failed");
  }
}

void BTHomeLoop()
{
  if (!BTHomeNextLoopTimestamp || !IsTimeout(BTHomeNextLoopTimestamp))
    return;

  if (BLECLientIsConnecting)
    return;

  if (!BLECLientIsConnected)
  {
    ESP_LOGI(BTHOMEV2_TAG, "trying to connect...");
    BLEClientConnect(settings.bthome_sensor_addr, connectCb, disconnectCb, readCb, readCb, rssiCb);
    BTHomeNextLoopTimestamp = GetTimestamp(BTHOME_CONNECTION_PERIOD_US);
  }
  else
  {
    BLEClientGetRssi();
    BTHomeNextLoopTimestamp = GetTimestamp(BTHOME_RSSI_READ_PERIOD_US);
  }
}

void connectCb()
{
  ESP_LOGI(BTHOMEV2_TAG, "connected");

  if (_connectCallback != nullptr)
    _connectCallback(true);

  BLEClientSetTransmitPower(ESP_PWR_LVL_P9);
  BLEClientGetRssi();

  //-----Ambient-----
  if (!BLEClientGetService(AMBIENT_SERVICE_ID, &btHomeAmbientService))
  {
    btHomeTemperatureChar.char_handle = 0;
    btHomeHumidityChar.char_handle = 0;
    ESP_LOGW(BTHOMEV2_TAG, "ambient service not found");
  }

  // temperature
  if (!BLEClientGetCharacteristic(AMBIENT_TEMPERATURE_CHAR_ID, btHomeAmbientService.start_handle, btHomeAmbientService.end_handle, &btHomeTemperatureChar))
  {
    btHomeTemperatureChar.char_handle = 0;
    ESP_LOGW(BTHOMEV2_TAG, "temperature characteristic not found");
  }
  else
  {
    BLEClientReadCharacteristic(btHomeTemperatureChar.char_handle);
    BLEClientRegisterForNotify(btHomeTemperatureChar.char_handle);
  }

  // humidity
  if (!BLEClientGetCharacteristic(AMBIENT_TEMPERATURE_HUMIDITY_CHAR_ID, btHomeAmbientService.start_handle, btHomeAmbientService.end_handle, &btHomeHumidityChar))
  {
    btHomeTemperatureChar.char_handle = 0;
    ESP_LOGW(BTHOMEV2_TAG, "humidity characteristic not found");
  }
  else
  {
    BLEClientReadCharacteristic(btHomeHumidityChar.char_handle);
    BLEClientRegisterForNotify(btHomeHumidityChar.char_handle);
  }

  //-----Battery-----
  if (!BLEClientGetService(BATTERY_SERVICE_ID, &btHomeBatteryService))
  {
    btHomeBatteryChar.char_handle = 0;
    ESP_LOGW(BTHOMEV2_TAG, "battery service not found");
  }

  if (!BLEClientGetCharacteristic(BATTERY_CHAR_ID, btHomeBatteryService.start_handle, btHomeBatteryService.end_handle, &btHomeBatteryChar))
  {
    btHomeBatteryChar.char_handle = 0;
    ESP_LOGW(BTHOMEV2_TAG, "battery characteristic not found");
  }
  else
  {
    BLEClientReadCharacteristic(btHomeBatteryChar.char_handle);
    BLEClientRegisterForNotify(btHomeBatteryChar.char_handle);
  }

  ESP_LOGI(BTHOMEV2_TAG, "characteristics readed");
}

void disconnectCb()
{
  ESP_LOGI(BTHOMEV2_TAG, "disconnected");

  BTHomeNextLoopTimestamp = GetTimestamp();

  if (_connectCallback != nullptr)
    _connectCallback(false);
}

void readCb(uint16_t handle, uint16_t length, uint8_t *value)
{
  ESP_LOGV(BTHOMEV2_TAG, "readCb, handle: 0x%04X", handle);

  if (btHomeTemperatureChar.char_handle && handle == btHomeTemperatureChar.char_handle)
  {
    if (length != 2)
    {
      ESP_LOGE(BTHOMEV2_TAG, "btHomeTemperatureChar bad data length %d", length);
      return;
    }

    ESP_LOGV(BTHOMEV2_TAG, "Temperature %f", BTHomeTemperature);

    if (_temperatureCallback != nullptr)
      _temperatureCallback((float)((value[1] << 8) + value[0]) * 0.01);
  }

  else if (btHomeTemperatureChar.char_handle && handle == btHomeHumidityChar.char_handle)
  {
    if (length != 2)
    {
      ESP_LOGE(BTHOMEV2_TAG, "btHomeHumidityChar bad data length %d", length);
      return;
    }

    ESP_LOGV(BTHOMEV2_TAG, "Humidity %f", BTHomeHumidity);

    if (_humidityCallback != nullptr)
      _humidityCallback((float)((value[1] << 8) + value[0]) * 0.01);
  }

  else if (btHomeBatteryChar.char_handle && handle == btHomeBatteryChar.char_handle)
  {
    if (length != 1)
    {
      ESP_LOGE(BTHOMEV2_TAG, "btHomeBatteryChar bad data length %d", length);
      return;
    }

    ESP_LOGV(BTHOMEV2_TAG, "Battery %d%", BTHomeBattery);

    if (_batteryCallback != nullptr)
      _batteryCallback(value[0]);
  }
}

void rssiCb(int8_t rssi)
{
  ESP_LOGV(BTHOMEV2_TAG, "RSSI %d", rssi);

  if (_rssiCallback != nullptr)
    _rssiCallback(rssi);
}