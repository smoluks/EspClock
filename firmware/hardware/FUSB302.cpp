#line 1 "C:\\Users\\Administrator\\Desktop\\clock_esp\\firmware\\hardware\\FUSB302.cpp"
//Based on FUSB302 PD UFP sink by Kai Liebich Version 0.1.0 https://github.com/kcl93/fusb302_arduino
//Please pay attention to performance
//We need to proceed events faster than 10ms
#include "h/FUSB302.hpp"
#include "../h/hardware.hpp"
#include "../managers/h/errorManager.hpp"
#include "../controllers/h/systickController.hpp"
#include "../hardware/h/i2c.hpp"
#include "../libs/FUSB302_PD_UFP_sink/src/FUSB302_UFP.h"
#include "../libs/FUSB302_PD_UFP_sink/src/PD_UFP_Protocol.h"

#define t_TypeCSinkWaitCap 350 //Time between SOP repeat
#define t_RequestToPSReady 580 // PD_PROTOCOL_EVENT_PS_RDY timeout after PD_PROTOCOL_EVENT_SRC_CAP. Combine t_SenderResponse and t_PSTransition
#define t_PPSRequest 5000      // time between PPS sync. Must less than 10000 (10s)

enum status_power_e
{
  FUSB302_STATUS_INITIAL = 0,
  FUSB302_STATUS_NA,
  FUSB302_STATUS_ANALOG,
  FUSB302_STATUS_PD,
  FUSB302_STATUS_PPS
} status_power;

void FUSB302InitInternal();
void handleFUSB302Event(FUSB302_event_t events);
void handleProtocolEvent(PD_protocol_event_t events);
void setPowerStatus(status_power_e status, uint16_t voltage, uint16_t current);

inline void readEvents();
inline void processSrcCap();
inline void processPPSRequest();
inline void processPDReadyTimeout();
inline void checkAnalogProtocol();

FUSB302_ret_t FUSB302_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t count);
FUSB302_ret_t FUSB302_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t count);
FUSB302_ret_t FUSB302_delay_ms(uint32_t t);

PD_protocol_t protocol;
FUSB302_dev_t FUSB302;
uint16_t ready_voltage;
uint16_t ready_current = 500;

inline void FUSB302Init()
{
  ESP_LOGI("FUSB302", "FUSB302 init started");

  status_power = FUSB302_STATUS_INITIAL;
  
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
    ESP_LOGE("FUSB302", "FUSB302 init return error %d", result);
    setError(ERROR_FUSB302_NOT_FOUND);
    return;
  }

  ESP_LOGV("FUSB302", "FUSB302 version 0x%X", FUSB302.reg_control[0]);

  // do only FUSB302 stuff to process pd init procedure with max performance
  timestamp_uS_t fusb_loop_timestamp = GetTimestamp(2000*1000);  
  do
  {
    FUSB302Loop();

    //need for watchdog reset
    delay(1);
  } while (!IsTimeout(fusb_loop_timestamp));

  ESP_LOGI("FUSB302", "PD init finished");
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

// mA
uint16_t GetPermittedUSBCurrent()
{
  return ready_current;
}

// read i2c regs events
inline void readEvents()
{
  FUSB302_event_t FUSB302_event_flags;
  FUSB302_event_flags = 0;
  if (FUSB302_alert(&FUSB302, &FUSB302_event_flags) != FUSB302_SUCCESS)
  {
    ESP_LOGE("FUSB302", "FUSB302_alert error");
    return;
  }

  if (FUSB302_event_flags)
  {
    handleFUSB302Event(FUSB302_event_flags);
  }
}

// Processing time between connection and start data exchange
bool send_src_cap;
timestamp_uS_t timestamp_src_cap;
uint8_t get_src_cap_retry_count;
inline void processSrcCap()
{
  if (!send_src_cap || !IsTimeout(timestamp_src_cap))
    return;

  if (++get_src_cap_retry_count == 5)
  {
    // No answer - stop polling
    send_src_cap = false;

    // FUSB302_tx_hard_reset(&FUSB302);
    // PD_protocol_reset(&protocol);
    checkAnalogProtocol();
    return;
  }

  /* Try to request soruce capabilities message (will not cause power cycle VBUS) */
  uint16_t header;
  PD_protocol_create_get_src_cap(&protocol, &header);
  FUSB302_tx_sop(&FUSB302, header, 0);

  timestamp_src_cap = GetTimestamp(t_TypeCSinkWaitCap*1000);

  ESP_LOGV("FUSB302", "Send SOP");
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

// Process PD request timeout (after host answer for src cap)
bool wait_pd_rdy;
timestamp_uS_t timestamp_wait_pd_rdy;
inline void processPDReadyTimeout()
{
  if (!wait_pd_rdy)
    return;

  if (!IsTimeout(timestamp_wait_pd_rdy))
    return;

  // not today
  wait_pd_rdy = false;
  checkAnalogProtocol();

  ESP_LOGW("FUSB302", "PD ready timeout");
}

// Send PPS request
timestamp_uS_t timestamp_next_PPS_request;
uint8_t send_PPS_request_now;
inline void processPPSRequest()
{
  if (wait_pd_rdy)
    return;

  if (send_PPS_request_now || (status_power == FUSB302_STATUS_PPS && IsTimeout(timestamp_next_PPS_request)))
  {
    wait_pd_rdy = true;
    timestamp_wait_pd_rdy = GetTimestamp(t_RequestToPSReady*1000);

    send_PPS_request_now = false;
    timestamp_next_PPS_request = GetTimestamp(t_PPSRequest*1000);

    uint16_t header;
    uint32_t obj[7];
    /* Send request if option updated or regularly in PPS mode to keep power alive */
    PD_protocol_create_request(&protocol, &header, obj);
    FUSB302_tx_sop(&FUSB302, header, obj);

    ESP_LOGV("FUSB302", "Sended PPS request");
  }
}

void handleFUSB302Event(FUSB302_event_t events)
{
  if (events & FUSB302_EVENT_DETACHED)
  {
    PD_protocol_reset(&protocol);
    setPowerStatus(FUSB302_STATUS_NA, 5000, 500);
    ESP_LOGV("FUSB302", "PD detached");
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
      ESP_LOGV("FUSB302", "Received RX_SOP Extended, message type 0x%X", (header >> 0) & 0x1F);
    }
    else if ((header >> 12) & 0x7)
    {
      ESP_LOGV("FUSB302", "Received RX_SOP Data, message type 0x%X", (header >> 0) & 0x1F);
    }
    else
    {
      ESP_LOGV("FUSB302", "Received RX_SOP Command, message type 0x%X", (header >> 0) & 0x1F);
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

    ESP_LOGV("FUSB302", "GOOD_CRC_SENT");
  }
}

uint16_t PPS_voltage_next;
uint8_t PPS_current_next;
void handleProtocolEvent(PD_protocol_event_t events)
{
  if (events & PD_PROTOCOL_EVENT_SRC_CAP)
  {
    // SRC_CAP answer received, no more sending SRC_CAP
    send_src_cap = false;

    // start timeout control
    wait_pd_rdy = true;
    timestamp_wait_pd_rdy = GetTimestamp(t_RequestToPSReady*1000);
    ESP_LOGV("FUSB302", "Received event PD_PROTOCOL_EVENT_SRC_CAP");
  }
  if (events & PD_PROTOCOL_EVENT_REJECT)
  {
    // thing is not in the mood to communicate - okay
    wait_pd_rdy = false;
    checkAnalogProtocol();

    ESP_LOGV("FUSB302", "Received event PD_PROTOCOL_EVENT_REJECT");
  }
  if (events & PD_PROTOCOL_EVENT_PS_RDY)
  {
    // success
    wait_pd_rdy = false;

    PD_power_info_t p;
    uint8_t i, selected_power = PD_protocol_get_selected_power(&protocol);
    PD_protocol_get_power_info(&protocol, selected_power, &p);
    if (p.type == PD_PDO_TYPE_AUGMENTED_PDO)
    {
      // PPS mode
      FUSB302_set_vbus_sense(&FUSB302, 0);
      if (PPS_voltage_next)
      {
        // Two stage startup for PPS voltage < 5V
        PD_protocol_set_PPS(&protocol, PPS_voltage_next, PPS_current_next, false);
        PPS_voltage_next = 0;

        ESP_LOGV("FUSB302", "Received event PD_PROTOCOL_EVENT_PS_RDY, make two-stage startup");
      }
      else
      {
        timestamp_next_PPS_request = millis();
        setPowerStatus(FUSB302_STATUS_PPS, PD_protocol_get_PPS_voltage(&protocol) * 20, PD_protocol_get_PPS_current(&protocol) * 50);

        // status_log_event(STATUS_LOG_POWER_READY);
        ESP_LOGV("FUSB302", "Received event PD_PROTOCOL_EVENT_PS_RDY, PPS OK");
      }
    }
    else
    {
      FUSB302_set_vbus_sense(&FUSB302, 1);
      setPowerStatus(FUSB302_STATUS_PD, p.max_v * 50, p.max_i * 10);

      ESP_LOGV("FUSB302", "Received event PD_PROTOCOL_EVENT_PS_RDY, PD OK");
    }
  }
}

/// voltage and current in mV, mA
void setPowerStatus(status_power_e status, uint16_t voltage, uint16_t current)
{
  status_power = status;
  ready_voltage = voltage;
  ready_current = current;

  switch (status)
  {
  case FUSB302_STATUS_NA:
    ESP_LOGI("FUSB302", "Not found any protocol in USB, default voltage: %d mV, current: %d mA", ready_voltage, ready_current);
    break;
  case FUSB302_STATUS_ANALOG:
    ESP_LOGI("FUSB302", "PD attached at analog mode with voltage: %d mV, current: %d mA", ready_voltage, ready_current);
    break;
  case FUSB302_STATUS_PD:
    ESP_LOGI("FUSB302", "PD attached at PD mode with voltage: %d mV, current: %d mA", ready_voltage, ready_current);
    break;
  case FUSB302_STATUS_PPS:
    ESP_LOGI("FUSB302", "PD attached at PPS mode with voltage: %d mV, current: %d mA", ready_voltage, ready_current);
    break;
  default:
    break;
  }
}

FUSB302_ret_t FUSB302_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t count)
{
  //ESP_LOGI("FUSB302", "Read addr: %X, count %d", reg_addr, count);

  size_t result =  I2CReadRegisters(dev_addr, reg_addr, data, count);
  
  //ESP_LOGI("FUSB302", "Read finished");

  return result == count ? FUSB302_SUCCESS : FUSB302_ERR_READ_DEVICE;
}

FUSB302_ret_t FUSB302_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t count)
{
  //ESP_LOGI("FUSB302", "Write addr: %X, count %d", reg_addr, count);

  size_t result =  I2CWriteRegisters(dev_addr, reg_addr, data, count);
  
  //ESP_LOGI("FUSB302", "Write finished");

  return result == count ? FUSB302_SUCCESS : FUSB302_ERR_WRITE_DEVICE;
}

FUSB302_ret_t FUSB302_delay_ms(uint32_t t)
{
  delay(t);

  return FUSB302_SUCCESS;
}
