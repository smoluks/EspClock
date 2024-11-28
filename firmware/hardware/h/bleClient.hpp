#pragma once

#include <esp_bt.h>
#include <esp_gatt_defs.h>

typedef void (* ble_client_callback)();
typedef void (* ble_client_data_callback)(uint16_t handle, uint16_t length, uint8_t* value);
typedef void (* ble_client_rssi_callback)(int8_t rssi);

bool BLECLientIsConnected;
bool BLECLientIsConnecting;

bool BLEClientInit();
bool BLEClientConnect(uint8_t addr[6],
                        ble_client_callback connect_callback,
                        ble_client_callback disconnect_callback,
                        ble_client_data_callback read_callback,
                        ble_client_data_callback notify_callback,
                        ble_client_rssi_callback rssi_callback);
bool BLEClientSetTransmitPower(esp_power_level_t power);
bool BLEClientGetRssi();
bool BLEClientGetService(uint16_t svc_uuid, esp_gattc_service_elem_t* result);
bool BLEClientGetCharacteristic(uint16_t char_uuid, uint16_t start_handle, uint16_t end_handle, esp_gattc_char_elem_t* result);
bool BLEClientReadCharacteristic(uint16_t handle);
bool BLEClientRegisterForNotify(uint16_t handle);