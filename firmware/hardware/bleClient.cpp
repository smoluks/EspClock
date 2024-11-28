// https://github.com/espressif/esp-idf/blob/master/examples/bluetooth/bluedroid/ble/gatt_client/tutorial/Gatt_Client_Example_Walkthrough.md
// https://github.com/espressif/esp-idf/blob/master/examples/bluetooth/bluedroid/ble/gatt_client/main/gattc_demo.c
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_gap_ble_api.h>
#include <esp_gattc_api.h>
#include <esp_gatt_common_api.h>
#include <esp_bt.h>
#include "h/BLEClient.hpp"
#include <nvs_flash.h>
#include <esp_gatts_api.h>

#define BLE_LOGI ESP_LOGI

static const char *BLE_CLIENT_TAG = "BLEClient";
static const char *DEVICE_NAME = "ESP Clock";

static void gapEventCallback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void gaccEventCallback(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
void processRegNotifyEvt(uint16_t char_handle);
static void processRegEvt(esp_gatt_if_t gattc_if, esp_gatt_status_t param);

// notify predefined descriptor
static const esp_bt_uuid_t notify_descr_uuid = {
    .len = ESP_UUID_LEN_16,
    .uuid = {
        .uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG,
    },
};


static ble_client_callback _bleClientConnectCallback;
static ble_client_callback _bleClientDisconnectCallback;
static ble_client_data_callback _bleClientReadCallback;
static ble_client_data_callback _bleClientNotifyCallback;
static ble_client_rssi_callback _bleClientRssiCallback;
static uint8_t bleClientRemoteAddr[6];
static esp_gatt_if_t bleClientInterface;
static uint16_t bleClientConnectionId;
static uint16_t bleClientAppId;

#pragma region Public methods

bool BLEClientInit()
{
    ESP_LOGI(BLE_CLIENT_TAG, "init started");
    esp_err_t errRc = ESP_OK;

    //i'm really fcked with init, so used arduino init
    if (!btStartMode(BT_MODE_BLE))
    {
        ESP_LOGE(BLE_CLIENT_TAG, "btStartMode error");
        return false;
    }

    /*errRc = nvs_flash_init();
    if (errRc != ESP_OK)
    {
        ESP_LOGW(BLE_CLIENT_TAG, "nvs_flash_init return error 0x%X", errRc);
    }

    esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
    esp_ble_gatts_app_register(1);

    //----controller init----
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    bt_cfg.mode = ESP_BT_MODE_BLE;

    /*errRc = esp_bt_controller_deinit();
    if (errRc != ESP_OK)
    {
        ESP_LOGW(BLE_CLIENT_TAG, "esp_bt_controller_deinit return error 0x%X", errRc);
    }

    esp_bt_controller_mem_release(ESP_BT_MODE_BLE);

    errRc = esp_bt_controller_init(&bt_cfg);
    if (errRc != ESP_OK)
        ESP_LOGW(BLE_CLIENT_TAG, "esp_bt_controller_init return error 0x%X", errRc);

    errRc = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (errRc != ESP_OK)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "esp_bt_controller_enable return error 0x%X", errRc);
        return false;
    }
    */

    //----stack init----
    esp_bluedroid_status_t bt_state = esp_bluedroid_get_status();
    if (bt_state == ESP_BLUEDROID_STATUS_UNINITIALIZED)
    {
        errRc = esp_bluedroid_init();
        if (errRc != ESP_OK)
        {
            ESP_LOGE(BLE_CLIENT_TAG, "esp_bluedroid_init return error 0x%X", errRc);
            return false;
        }

        bt_state = esp_bluedroid_get_status();
    }

    if (bt_state != ESP_BLUEDROID_STATUS_ENABLED)
    {
        errRc = esp_bluedroid_enable();
        if (errRc != ESP_OK)
        {
            ESP_LOGE(BLE_CLIENT_TAG, "esp_bluedroid_enable return error 0x%X", errRc);
            return false;
        }
    }

    //----callback register-----
    errRc = esp_ble_gap_register_callback(gapEventCallback);
    if (errRc != ESP_OK)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "esp_ble_gap_register_callback return error 0x%X", errRc);
        return false;
    }

    errRc = esp_ble_gattc_register_callback(gaccEventCallback);
    if (errRc != ESP_OK)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "esp_ble_gattc_register_callback return error 0x%X", errRc);
        return false;
    }

    //
    errRc = esp_ble_gap_set_device_name(DEVICE_NAME);
    if (errRc != ESP_OK)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "esp_ble_gap_set_device_name return error 0x%X", errRc);
        return false;
    };

#ifdef CONFIG_BLE_SMP_ENABLE // Check that BLE SMP (security) is configured in make menuconfig
    esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;
    errRc = esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
    if (errRc)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "esp_ble_gap_set_security_param return error 0x%X", errRc);
        return false;
    };
#endif // CONFIG_BLE_SMP_ENABLE

    ESP_LOGI(BLE_CLIENT_TAG, "init sucessfully finished");

    return true;
}

bool BLEClientConnect(uint8_t addr[6],
                        ble_client_callback connect_callback,
                        ble_client_callback disconnect_callback,
                        ble_client_data_callback read_callback,
                        ble_client_data_callback notify_callback,
                        ble_client_rssi_callback rssi_callback)
{
    if (BLECLientIsConnected)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "already connected");
        return ESP_FAIL;
    }

    if (BLECLientIsConnecting)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "still in previous connecting procedure");
        return ESP_FAIL;
    }

    memcpy(bleClientRemoteAddr, addr, 6);
    _bleClientConnectCallback = connect_callback;
    _bleClientDisconnectCallback = disconnect_callback;
    _bleClientReadCallback = read_callback;
    _bleClientNotifyCallback = notify_callback;
    _bleClientRssiCallback = rssi_callback;
    BLECLientIsConnecting = true;

    //appId += 1;
    esp_err_t errRc = esp_ble_gattc_app_register(bleClientAppId);
    if (errRc)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "esp_ble_gattc_app_register return error 0x%X", errRc);
        return false;
    };
    BLE_LOGI(BLE_CLIENT_TAG, "app registered");

    return true;
}

bool BLEClientSetTransmitPower(esp_power_level_t power)
{
    if (!BLECLientIsConnected)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "not connected");
        return ESP_FAIL;
    }

    esp_err_t errRc = esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, power);
    if (errRc)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "esp_ble_tx_power_set return error 0x%X", errRc);
        return false;
    };

    return true;
}

bool BLEClientGetRssi()
{
    if (!BLECLientIsConnected)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "not connected");
        return ESP_FAIL;
    }

    esp_err_t errRc = esp_ble_gap_read_rssi(bleClientRemoteAddr);
    if (errRc)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "esp_ble_gap_read_rssi return error 0x%X", errRc);
        return false;
    };

    return true;
}

bool BLEClientGetService(uint16_t svc_uuid, esp_gattc_service_elem_t *result)
{
    if (!BLECLientIsConnected)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "not connected");
        return false;
    }

    esp_bt_uuid_t remoteFilterServiceUuid = {
        .len = ESP_UUID_LEN_16,
        .uuid{
            .uuid16 = svc_uuid,
        },
    };
    uint16_t count = 1;
    esp_err_t errRc = esp_ble_gattc_get_service(bleClientInterface, bleClientConnectionId, &remoteFilterServiceUuid, result, &count, 0);
    if (errRc)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "esp_ble_gattc_get_service return error 0x%X", errRc);
        return false;
    };
    if (!count)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "service not found");
        return false;
    };

    return true;
}

bool BLEClientGetCharacteristic(uint16_t char_uuid, uint16_t start_handle, uint16_t end_handle, esp_gattc_char_elem_t *result)
{
    if (!BLECLientIsConnected)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "not connected");
        return false;
    }

    esp_bt_uuid_t uuid = {
        .len = ESP_UUID_LEN_16,
        .uuid{
            .uuid16 = char_uuid,
        },
    };
    uint16_t count = 1;
    esp_err_t errRc = esp_ble_gattc_get_char_by_uuid(bleClientInterface, bleClientConnectionId, start_handle, end_handle, uuid, result, &count);
    if (errRc)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "esp_ble_gattc_get_char_by_uuid return error 0x%X", errRc);
        return false;
    };
    if (!count)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "characteristic not found");
        return false;
    };

    return true;
}

bool BLEClientReadCharacteristic(uint16_t handle)
{
    if (!BLECLientIsConnected)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "not connected");
        return false;
    }

    esp_err_t errRc = esp_ble_gattc_read_char(bleClientInterface, bleClientConnectionId, handle, ESP_GATT_AUTH_REQ_NONE);
    if (errRc)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "esp_ble_gattc_read_char return error 0x%X", errRc);
        return false;
    };

    return true;
}

bool BLEClientRegisterForNotify(uint16_t handle)
{
    if (!BLECLientIsConnected)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "not connected");
        return false;
    }

    esp_err_t errRc = esp_ble_gattc_register_for_notify(bleClientInterface, bleClientRemoteAddr, handle);
    if (errRc)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "esp_ble_gattc_register_for_notify return error 0x%X", errRc);
        return false;
    };

    return true;
}

#pragma endregion

#pragma region Private methods

static void gapEventCallback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
    // set pkt length complete
    case ESP_GAP_BLE_SET_PKT_LENGTH_COMPLETE_EVT:
        if (param->pkt_data_length_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(BLE_CLIENT_TAG, "set pkt length status error %d", param->pkt_data_length_cmpl.status);
            return;
        }

        BLE_LOGI(BLE_CLIENT_TAG, "set pkt length complete");
        break;

    // read the rssi complete
    case ESP_GAP_BLE_READ_RSSI_COMPLETE_EVT:
        if (param->read_rssi_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(BLE_CLIENT_TAG, "read rssi status error %d", param->read_rssi_cmpl.status);
            return;
        }

        BLE_LOGI(BLE_CLIENT_TAG, "read rssi finished with value %d", param->read_rssi_cmpl.rssi);
        if (_bleClientRssiCallback != nullptr)
        {
            _bleClientRssiCallback(param->read_rssi_cmpl.rssi);
        }
        break;

    // update connection parameters complete
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        if (param->update_conn_params.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(BLE_CLIENT_TAG, "update conn params status error %d", param->update_conn_params.status);
            return;
        }

        BLE_LOGI(BLE_CLIENT_TAG, "update connection parameters finished with status: %d and connection interval %d", param->update_conn_params.status, param->update_conn_params.conn_int);
        BLECLientIsConnected = true;
        break;

    default:
        ESP_LOGI(BLE_CLIENT_TAG, "gap_event_callback: %d", event);
        break;
    }
}

static void gaccEventCallback(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    esp_err_t errRc;

    // order as on connection
    switch (event)
    {
    //-----Connection-----
    // GATT client is registered
    case ESP_GATTC_REG_EVT:
        BLE_LOGI(BLE_CLIENT_TAG, "GATT client is registered with app_id %d", param->reg.app_id);
        processRegEvt(gattc_if, param->reg.status);
        break;

    // ble physical connection is set up
    case ESP_GATTC_CONNECT_EVT:
        BLE_LOGI(BLE_CLIENT_TAG, "ble physical connection is set up with role %d", param->connect.link_role);
        break;

    // GATT virtual connection is set up
    case ESP_GATTC_OPEN_EVT:
        // GATT virtual connection is set up
        BLE_LOGI(BLE_CLIENT_TAG, "GATT virtual connection is set up with MTU %d", param->open.mtu);
        bleClientInterface = gattc_if;

        if (!param->open.mtu)
        {
            esp_ble_gattc_app_unregister(gattc_if);
            BLE_LOGI(BLE_CLIENT_TAG, "app unregistered on interface %d", gattc_if);
        }

        break;

    //-----Service discovery-----
    // the ble discover service complete
    case ESP_GATTC_DIS_SRVC_CMPL_EVT:
        BLE_LOGI(BLE_CLIENT_TAG, "ble discover service complete with status %d and conn_id %d, start services discovery", param->dis_srvc_cmpl.status, param->dis_srvc_cmpl.conn_id);
        bleClientConnectionId = param->dis_srvc_cmpl.conn_id;
        errRc = esp_ble_gattc_search_service(bleClientInterface, param->dis_srvc_cmpl.conn_id, NULL);
        if (errRc != ESP_OK)
        {
            ESP_LOGE(BLE_CLIENT_TAG, "esp_ble_gattc_search_service return error 0x%X", errRc);
        }
        break;

    // GATT service discovery result is got
    case ESP_GATTC_SEARCH_RES_EVT:
        BLE_LOGI(BLE_CLIENT_TAG, "ble service discovery result: uuid 0x%X", param->search_res.srvc_id.uuid);
        break;

    // GATT service discovery is completed
    case ESP_GATTC_SEARCH_CMPL_EVT:
        BLE_LOGI(BLE_CLIENT_TAG, "GATT service discovery is completed");
        BLECLientIsConnecting = false;
        BLECLientIsConnected = true;

        if (_bleClientConnectCallback != nullptr)
            _bleClientConnectCallback();

        break;

    //-----Data-----
    // When GATT characteristic is read
    case ESP_GATTC_READ_CHAR_EVT:
        BLE_LOGI(BLE_CLIENT_TAG, "GATT characteristic is read, handle 0x%04X", param->read.handle);
        if (_bleClientReadCallback != nullptr)
        {
            _bleClientReadCallback(param->read.handle, param->read.value_len, param->read.value);
        }
        break;

    //-----Notifications-----
    // register for notification of a service completes
    case ESP_GATTC_REG_FOR_NOTIFY_EVT:
        BLE_LOGI(BLE_CLIENT_TAG, "register for notification of a service completes, handle 0x%04X", param->reg_for_notify.handle);

        processRegNotifyEvt(param->reg_for_notify.handle);
        break;

    // When GATT notification or indication arrives
    case ESP_GATTC_NOTIFY_EVT:
        BLE_LOGI(BLE_CLIENT_TAG, "GATT notification or indication arrives, handle 0x%04X, length %d", param->notify.handle, param->notify.value_len);
        if (_bleClientNotifyCallback != nullptr)
            _bleClientNotifyCallback(param->notify.handle, param->notify.value_len, param->notify.value);
        break;

    //-----Write-----
    // GATT characteristic descriptor write completes
    case ESP_GATTC_WRITE_DESCR_EVT:
        BLE_LOGI(BLE_CLIENT_TAG, "GATT characteristic descriptor write completes, handle 0x%04X", param->write.handle);
        break;

    //-----Disconnect-----
    // the ble physical connection disconnected
    case ESP_GATTC_DISCONNECT_EVT:
        BLE_LOGI(BLE_CLIENT_TAG, "ble physical connection disconnected with reason 0x%02x", param->disconnect.reason);

        if (bleClientConnectionId)
        {
            errRc = esp_ble_gattc_close(gattc_if, bleClientConnectionId);
            if (errRc != ESP_OK)
            {
                ESP_LOGE(BLE_CLIENT_TAG, "esp_ble_gattc_close return error 0x%X", errRc);
            }
            BLE_LOGI(BLE_CLIENT_TAG, "closed connection on interface %d and app_id %d", gattc_if, 1);

            bleClientConnectionId = 0;
        }

        break;

    // GATT virtual connection is closed
    case ESP_GATTC_CLOSE_EVT:
        BLE_LOGI(BLE_CLIENT_TAG, "GATT virtual connection is closed with reason 0x%X", param->close.reason);

        BLECLientIsConnected = false;
        if (_bleClientDisconnectCallback != nullptr)
            _bleClientDisconnectCallback();
        break;

    // GATT client is unregistered
    case ESP_GATTC_UNREG_EVT:
        BLE_LOGI(BLE_CLIENT_TAG, "GATT client is unregistered");
        BLECLientIsConnecting = false;
        break;

    default:
        ESP_LOGI(BLE_CLIENT_TAG, "gacc_event_callback: %d, if %d", event, gattc_if);
        break;
    }
}

void processRegNotifyEvt(uint16_t char_handle)
{
    esp_gattc_descr_elem_t notifyDescriptor;
    uint16_t count = 1;
    esp_gatt_status_t retStatus = esp_ble_gattc_get_descr_by_char_handle(bleClientInterface,
                                                                          bleClientConnectionId,
                                                                          char_handle,
                                                                          notify_descr_uuid,
                                                                          &notifyDescriptor,
                                                                          &count);
    if (retStatus != ESP_GATT_OK)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "esp_ble_gattc_get_descr_by_char_handle return error 0x%X", retStatus);
        return;
    }
    if (!count)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "notify descriptor not found in service 0x%X", char_handle);
        return;
    }

    uint16_t notifyEnableValue = 1;
    esp_err_t ret = esp_ble_gattc_write_char_descr(bleClientInterface,
                                                   bleClientConnectionId,
                                                   notifyDescriptor.handle,
                                                   sizeof(notifyEnableValue),
                                                   (uint8_t *)&notifyEnableValue,
                                                   ESP_GATT_WRITE_TYPE_RSP,
                                                   ESP_GATT_AUTH_REQ_NONE);
    if (ret != ESP_OK)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "esp_ble_gattc_write_char_descr return error 0x%X", ret);
        return;
    }
}

// Connection handler
static void processRegEvt(esp_gatt_if_t gattc_if, esp_gatt_status_t param)
{
    if (param != ESP_GATT_OK)
    {
        // connect failed
        BLE_LOGI(BLE_CLIENT_TAG, "process_reg_evt param error: 0x%X", param);
        //esp_ble_gattc_app_unregister(gattc_if);

        BLECLientIsConnecting = false;
        return;
    }

    esp_err_t errRc = esp_ble_gattc_open(gattc_if, bleClientRemoteAddr, BLE_ADDR_TYPE_PUBLIC, 1);
    if (errRc != ESP_OK)
    {
        ESP_LOGE(BLE_CLIENT_TAG, "esp_ble_gattc_open return error 0x%X", errRc);
    }
    BLE_LOGI(BLE_CLIENT_TAG, "open connection on interface %d", gattc_if);
}

#pragma endregion