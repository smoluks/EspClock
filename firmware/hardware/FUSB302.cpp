//Based on FUSB302 PD UFP sink by Kai Liebich Version 0.1.0 https://github.com/kcl93/fusb302_arduino
//Please pay attention to performance
//We need to proceed events faster than 10ms
#include "h/FUSB302.hpp"
#include "../h/hardware.hpp"
#include "../managers/h/errorManager.hpp"
#include "../controllers/h/systick.hpp"
#include "../hardware/h/i2c.hpp"
#include "../libs/FUSB302_PD_UFP_sink/src/FUSB302_UFP.h"
#include "../libs/FUSB302_PD_UFP_sink/src/PD_UFP_Protocol.h"

#define t_TypeCSinkWaitCap 350 //Time between SOP repeat
#define t_RequestToPSReady 580 // PD_PROTOCOL_EVENT_PS_RDY timeout after PD_PROTOCOL_EVENT_SRC_CAP. Combine t_SenderResponse and t_PSTransition
#define t_PPSRequest 5000      // time between PPS sync. Must less than 10000 (10s)

static const char *FUSB302_TAG = "FUSB302";

void (*PowerChangedHandler)(status_power_e status, uint16_t voltage, uint16_t current) = nullptr;

static void FUSB302InitInternal();
static void handleFUSB302Event(FUSB302_event_t events);
static void handleProtocolEvent(PD_protocol_event_t events);
static void setPowerStatus(status_power_e status, uint16_t voltage, uint16_t current);

static inline void readEvents();
static inline void processSrcCap();
static inline void processPPSRequest();
static inline void processPDReadyTimeout();
static inline void checkAnalogProtocol();

static FUSB302_ret_t FUSB302_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t count);
static FUSB302_ret_t FUSB302_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t count);
static FUSB302_ret_t FUSB302_delay_ms(uint32_t t);

static PD_protocol_t protocol;
static FUSB302_dev_t FUSB302;

inline void FUSB302Init()
{
  ESP_LOGI(FUSB302_TAG, "FUSB302 init started");

  memset(&FUSB302, 0, sizeof(FUSB302_dev_t));
  memset(&protocol, 0, sizeof(PD_protocol_t));

  // Initialize PD protocol engine
  PD_protocol_init(&protocol);
  PD_protocol_set_power_option(&protocol, PD_POWER_OPTION_MAX_12V);

  // this->int_pin = int_pin;
  //attachInterrupt(FUSB302_INT_PIN, FUSB302Interrupt, FALLING);
  pinMode(FUSB302_INT_PIN, INPUT_PULLUP); // Set FUSB302 int pin input ant pull up

  // Initialize FUSB302
  FUSB302.i2c_address = 0x22;
  FUSB302.i2c_read = FUSB302_i2c_read;
  FUSB302.i2c_write = FUSB302_i2c_write;
  FUSB302.delay_ms = FUSB302_delay_ms;

  uint8_t count;
  FUSB302_ret_t result;
  do
  {
    result = FUSB302_init(&FUSB302);
    if(result != FUSB302_SUCCESS)
          delay(100);
  } while (result != FUSB302_SUCCESS && ++count < 10);
  if (result != FUSB302_SUCCESS)
  {
    ESP_LOGE(FUSB302_TAG, "FUSB302 init return error %d", result);
    setError(ERROR_FUSB302_NOT_FOUND);
    return;
  }

  ESP_LOGI(FUSB302_TAG, "FUSB302 version 0x%X", FUSB302.reg_control[0]);

  // do only FUSB302 stuff to process pd init procedure with max performance
  timestamp_uS_t fusb_loop_timestamp = GetTimestamp(2000*1000);  
  do
  {
    FUSB302Loop();

    //need for watchdog reset
    delay(1);
  } while (!IsTimeout(fusb_loop_timestamp));

  ESP_LOGI(FUSB302_TAG, "PD init finished");
}

bool first_time = true;
inline void FUSB302Loop()
{
  if(first_time || !digitalRead(FUSB302_INT_PIN))
  {
    readEvents();
    first_time = false;
  }

  processSrcCap();
  processPDReadyTimeout();
  processPPSRequest();
}

// read i2c regs events
inline void readEvents()
{
  FUSB302_event_t FUSB302_event_flags;
  FUSB302_event_flags = 0;
  if (FUSB302_alert(&FUSB302, &FUSB302_event_flags) != FUSB302_SUCCESS)
  {
    ESP_LOGE(FUSB302_TAG, "FUSB302_alert error");
    return;
  }

  if (FUSB302_event_flags)
  {
    handleFUSB302Event(FUSB302_event_flags);
  }
}

//6.4.1.2 Source_Capabilities Message
static bool send_src_cap;
static timestamp_uS_t timestamp_src_cap;
static uint8_t get_src_cap_retry_count;
inline void processSrcCap()
{
  // Processing time between connection and starting data exchange
  if (!send_src_cap || !IsTimeout(timestamp_src_cap))
    return;

  if (++get_src_cap_retry_count == 5)
  {
    // No answer - stop polling
    send_src_cap = false;

    // FUSB302_tx_hard_reset(&FUSB302);
    PD_protocol_reset(&protocol);
    checkAnalogProtocol();
    return;
  }

  //Try to request soruce capabilities message (will not cause power cycle VBUS)
  uint16_t header;
  PD_protocol_create_get_src_cap(&protocol, &header);
  FUSB302_tx_sop(&FUSB302, header, 0);

  timestamp_src_cap = GetTimestamp(t_TypeCSinkWaitCap*1000);

  ESP_LOGV(FUSB302_TAG, "Send SOP");
}

inline void checkAnalogProtocol()
{
  /*  00: < 200 mV          : vRa
      01: >200 mV, <660 mV  : vRd-USB
      10: >660 mV, <1.23 V  : vRd-1.5
      11: >1.23 V           : vRd-3.0  */
  uint8_t cc1 = 0, cc2 = 0;
  FUSB302_get_cc(&FUSB302, &cc1, &cc2);

  uint8_t cc;
  if (cc1 == 0)
    cc = cc2;
  else if (cc2 == 0)
    cc = cc1;
  else
  {
    //ESP_LOGW("FUSB302", "Cable has two CC: CC1 %d, CC2 %d", cc1, cc2);
    setPowerStatus(FUSB302_STATUS_NA, 5000, 500);
    return;
  }

  switch (cc)
  {
  case 2:
    setPowerStatus(FUSB302_STATUS_ANALOG, 5000, 1500);
    break;
  case 3:
    setPowerStatus(FUSB302_STATUS_ANALOG, 5000, 3000);
    break;
  case 1:
    setPowerStatus(FUSB302_STATUS_ANALOG, 5000, 500);
    break;
  default:
    setPowerStatus(FUSB302_STATUS_NA, 5000, 500);
    break;
  }
}

// Check PD profile request timeout
static bool wait_ps_rdy;
static timestamp_uS_t timestamp_wait_pd_rdy;
inline void processPDReadyTimeout()
{
  if (!wait_ps_rdy || !IsTimeout(timestamp_wait_pd_rdy))
    return;

  // not today
  wait_ps_rdy = false;
  checkAnalogProtocol();

  ESP_LOGW(FUSB302_TAG, "PS ready timeout");
}

// Send PPS request - not used
static timestamp_uS_t timestamp_next_PPS_request;
static uint8_t is_PPS_used = false;
inline void processPPSRequest()
{
  if (is_PPS_used && !wait_ps_rdy && IsTimeout(timestamp_next_PPS_request))
  {
    wait_ps_rdy = true;
    timestamp_wait_pd_rdy = GetTimestamp(t_RequestToPSReady*1000);

    timestamp_next_PPS_request = GetTimestamp(t_PPSRequest*1000);

    uint16_t header;
    uint32_t obj[7];
    /* Send request if option updated or regularly in PPS mode to keep power alive */
    PD_protocol_create_request(&protocol, &header, obj);
    FUSB302_tx_sop(&FUSB302, header, obj);

    ESP_LOGV(FUSB302_TAG, "Sended PPS request");
  }
}

void handleFUSB302Event(FUSB302_event_t events)
{
  if (events & FUSB302_EVENT_DETACHED)
  {
    PD_protocol_reset(&protocol);
    setPowerStatus(FUSB302_STATUS_NA, 5000, 500);
    ESP_LOGV(FUSB302_TAG, "PD detached");
  }
  if (events & FUSB302_EVENT_ATTACHED)
  {
    uint8_t cc1 = 0, cc2 = 0;
    FUSB302_get_cc(&FUSB302, &cc1, &cc2);
    ESP_LOGV("FUSB302", "PD attached, CC1 %d, CC2 %d", cc1, cc2);

    if (cc1 > 1 || cc2 > 1)
    {
      // let's try to exchange
      send_src_cap = true;
      timestamp_src_cap = GetTimestamp(t_TypeCSinkWaitCap*1000);
      get_src_cap_retry_count = 0;
    }
    else
      checkAnalogProtocol();
  }
  if (events & FUSB302_EVENT_RX_SOP)
  {
    PD_protocol_event_t protocol_event = 0;
    uint16_t header;
    uint32_t obj[7];

    FUSB302_get_message(&FUSB302, &header, obj);

    PD_protocol_handle_msg(&protocol, header, obj, &protocol_event);
    if (protocol_event)
    {
      handleProtocolEvent(protocol_event);
    }
    /*
    if ((header >> 15) & 0x1)
    {
      ESP_LOGV(FUSB302_TAG, "Received RX_SOP Extended, message type 0x%X", (header >> 0) & 0x1F);
    }
    else if ((header >> 12) & 0x7)
    {
      ESP_LOGV(FUSB302_TAG, "Received RX_SOP Data, message type 0x%X", (header >> 0) & 0x1F);
    }
    else
    {
      ESP_LOGV(FUSB302_TAG, "Received RX_SOP Command, message type 0x%X", (header >> 0) & 0x1F);
    }*/
  }
  if (events & FUSB302_EVENT_GOOD_CRC_SENT)
  {
    uint16_t header;
    uint32_t obj[7];
    //delay(2); /* Delay respond in case there are retry messages */
    if (PD_protocol_respond(&protocol, &header, obj))
    {
      FUSB302_tx_sop(&FUSB302, header, obj);
    }

    ESP_LOGV(FUSB302_TAG, "GOOD_CRC_SENT");
  }
}

uint16_t PPS_voltage_next;
uint8_t PPS_current_next;
void handleProtocolEvent(PD_protocol_event_t events)
{
  if (events & PD_PROTOCOL_EVENT_SRC_CAP) //SRC_CAP answer received
  {
    //no more sending SRC_CAP
    send_src_cap = false;

    //TODO: check returned profiles and select most interesting for us 

    // start ready timeout control because library already answered with our profile 
    wait_ps_rdy = true;

    timestamp_wait_pd_rdy = GetTimestamp(t_RequestToPSReady*1000);
    ESP_LOGV(FUSB302_TAG, "Received event PD_PROTOCOL_EVENT_SRC_CAP");
  }
  if (events & PD_PROTOCOL_EVENT_REJECT) // not supported requested profile - okay
  {
    wait_ps_rdy = false;
    checkAnalogProtocol();

    ESP_LOGV(FUSB302_TAG, "Received event PD_PROTOCOL_EVENT_REJECT");
  }
  if (events & PD_PROTOCOL_EVENT_PS_RDY) // requested profile started
  {
    wait_ps_rdy = false;

    PD_power_info_t p;
    uint8_t i, selected_power = PD_protocol_get_selected_power(&protocol);
    PD_protocol_get_power_info(&protocol, selected_power, &p);
    if (p.type == PD_PDO_TYPE_AUGMENTED_PDO)
    {
      // PPS mode - not used
      is_PPS_used = true;

      FUSB302_set_vbus_sense(&FUSB302, 0);
      if (PPS_voltage_next)
      {
        // Two stage startup for PPS voltage < 5V
        PD_protocol_set_PPS(&protocol, PPS_voltage_next, PPS_current_next, false);
        PPS_voltage_next = 0;

        ESP_LOGV(FUSB302_TAG, "Received event PD_PROTOCOL_EVENT_PS_RDY, make two-stage startup");
      }
      else
      {
        timestamp_next_PPS_request = GetTimestamp(); //now!

        setPowerStatus(FUSB302_STATUS_PPS, PD_protocol_get_PPS_voltage(&protocol) * 20, PD_protocol_get_PPS_current(&protocol) * 50);

        ESP_LOGV(FUSB302_TAG, "Received event PD_PROTOCOL_EVENT_PS_RDY");
      }
    }
    else
    {
      //Profile mode
      is_PPS_used = false;
      FUSB302_set_vbus_sense(&FUSB302, 1);
      setPowerStatus(FUSB302_STATUS_PD, p.max_v * 50, p.max_i * 10);

      ESP_LOGV(FUSB302_TAG, "Received event PD_PROTOCOL_EVENT_PS_RDY, PD OK");
    }
  }
}

/// voltage and current in mV, mA
void setPowerStatus(status_power_e status, uint16_t ready_voltage, uint16_t ready_current)
{
  if(PowerChangedHandler != nullptr)
    PowerChangedHandler(status, ready_voltage, ready_current);

  switch (status)
  {
  case FUSB302_STATUS_NA:
    ESP_LOGI(FUSB302_TAG, "Not found any protocol in USB, default voltage: %d mV, current: %d mA", ready_voltage, ready_current);
    break;
  case FUSB302_STATUS_ANALOG:
    ESP_LOGI(FUSB302_TAG, "PD attached at analog mode with voltage: %d mV, current: %d mA", ready_voltage, ready_current);
    break;
  case FUSB302_STATUS_PD:
    ESP_LOGI(FUSB302_TAG, "PD attached at PD mode with voltage: %d mV, current: %d mA", ready_voltage, ready_current);
    break;
  case FUSB302_STATUS_PPS:
    ESP_LOGI(FUSB302_TAG, "PD attached at PPS mode with voltage: %d mV, current: %d mA", ready_voltage, ready_current);
    break;
  default:
    break;
  }
}

inline FUSB302_ret_t FUSB302_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t count)
{
  //ESP_LOGI(FUSB302_TAG, "Read addr: %X, count %d", reg_addr, count);

  size_t result =  I2CReadRegisters(dev_addr, reg_addr, data, count);
  
  //ESP_LOGI(FUSB302_TAG, "Read finished");

  return result == count ? FUSB302_SUCCESS : FUSB302_ERR_READ_DEVICE;
}

inline FUSB302_ret_t FUSB302_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t count)
{
  //ESP_LOGI(FUSB302_TAG, "Write addr: %X, count %d", reg_addr, count);

  size_t result =  I2CWriteRegisters(dev_addr, reg_addr, data, count);
  
  //ESP_LOGI(FUSB302_TAG, "Write finished");

  return result == count ? FUSB302_SUCCESS : FUSB302_ERR_WRITE_DEVICE;
}

inline FUSB302_ret_t FUSB302_delay_ms(uint32_t t)
{
  delay(t);

  return FUSB302_SUCCESS;
}
