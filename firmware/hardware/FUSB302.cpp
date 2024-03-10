#include "PD_UFP.h"
#include "FUSB302_UFP.h"
#include "h/hub75.hpp"
#include "h/fusb302.hpp"
#include "../h/pinsMapping.hpp"
#include "../managers/h/errorManager.hpp"

#define t_PD_POLLING 100
#define t_TypeCSinkWaitCap 350
#define t_RequestToPSReady 580 // combine t_SenderResponse and t_PSTransition
#define t_PPSRequest 5000      // must less than 10000 (10s)

PD_protocol_t protocol;
FUSB302_dev_t FUSB302;
status_power_t status_power;
uint16_t permitted_current = 500; // in mA

void FUSB302InitInternal();
void FUSB302LoopInternal();
void handle_FUSB302_event(FUSB302_event_t events);
void set_default_power(void);
void handle_protocol_event(PD_protocol_event_t events);
void status_power_ready(status_power_t status, uint16_t voltage, uint16_t current);
FUSB302_ret_t FUSB302_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t count);
FUSB302_ret_t FUSB302_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t count);
FUSB302_ret_t FUSB302_delay_ms(uint32_t t);

void FUSB302Init()
{
  ESP_LOGI("FUSB302", "FUSB302 init started");

  FUSB302InitInternal();

  ESP_LOGI("FUSB302", "PD init finished");
}

void FUSB302Loop()
{
  FUSB302LoopInternal();
}

uint16_t FUSB302GetPermittedCurrent()
{
  return permitted_current;
}

bool IsFUSB302PDPresent()
{
  return status_power != STATUS_POWER_NA;
}

//-----private-----
// Timer and counter for PD Policy
uint16_t time_polling;
uint16_t time_wait_src_cap;
uint16_t time_wait_ps_rdy;
uint16_t time_PPS_request;
uint8_t get_src_cap_retry_count;
uint8_t wait_src_cap;
uint8_t wait_ps_rdy;
uint8_t send_request;
static uint8_t clock_prescaler;
// PPS setup
uint16_t PPS_voltage_next;
uint8_t PPS_current_next;
// Power ready power
uint16_t ready_voltage;
uint16_t ready_current;

inline void FUSB302InitInternal()
{
  memset(&FUSB302, 0, sizeof(FUSB302_dev_t));
  memset(&protocol, 0, sizeof(PD_protocol_t));

  // Initialize PD protocol engine
  PD_protocol_init(&protocol);
  PD_protocol_set_power_option(&protocol, PD_POWER_OPTION_MAX_12V);

  // this->int_pin = int_pin;
  //  attachInterrupt(FUSB302_INT_PIN, FUSB302Interrupt, FALLING);
  pinMode(FUSB302_INT_PIN, INPUT_PULLUP); // Set FUSB302 int pin input ant pull up

  // Initialize FUSB302
  FUSB302.i2c_address = 0x22;
  FUSB302.i2c_read = FUSB302_i2c_read;
  FUSB302.i2c_write = FUSB302_i2c_write;
  FUSB302.delay_ms = FUSB302_delay_ms;

  FUSB302_ret_t result = FUSB302_init(&FUSB302);
  if (result != FUSB302_SUCCESS)
  {
    ESP_LOGE("FUSB302", "FUSB302 init return error %d", result);
    setError(ERROR_FUSB302_NOT_FOUND);
    return;
  }

  ESP_LOGI("FUSB302", "FUSB302 version 0x%X", FUSB302.reg_control[0]);
}

inline void FUSB302LoopInternal(void)
{
  //-----read events from fusb302-----
  // if (timer() || digitalRead(int_pin) == 0) {
  FUSB302_event_t FUSB302_event_flags;
  do
  {
    if(wait_src_cap)
      ESP_LOGI("FUSB302", "Cycle");

    FUSB302_event_flags = 0;
    if (FUSB302_alert(&FUSB302, &FUSB302_event_flags) != FUSB302_SUCCESS)
    {
      ESP_LOGE("FUSB302", "FUSB302_alert error");
      return;
    }
    if (FUSB302_event_flags)
    {
      handle_FUSB302_event(FUSB302_event_flags);
    }
  } while (FUSB302_event_flags != 0);

  //-----wait_src_cap processing-----
  if (wait_src_cap && millis() - time_wait_src_cap > t_TypeCSinkWaitCap)
  {
    time_wait_src_cap = millis();
    if (get_src_cap_retry_count < 5)
    {
      uint16_t header;
      get_src_cap_retry_count += 1;

      /* Try to request soruce capabilities message (will not cause power cycle VBUS) */
      PD_protocol_create_get_src_cap(&protocol, &header);
      // status_log_event(STATUS_LOG_MSG_TX);
      FUSB302_tx_sop(&FUSB302, header, 0);

      ESP_LOGI("FUSB302", "Send SOP");
    }
    else
    {
      /*
      get_src_cap_retry_count = 0;
      /* Hard reset will cause the source power cycle VBUS. */
      // FUSB302_tx_hard_reset(&FUSB302);
      // PD_protocol_reset(&protocol);

      // ESP_LOGI("FUSB302", "Send hard reset");
    }
  }

  if (wait_ps_rdy)
  {
    if (millis() - time_wait_ps_rdy > t_RequestToPSReady)
    {
      wait_ps_rdy = 0;
      set_default_power();

      ESP_LOGI("FUSB302", "Set PS ready");
    }
  }
  else if (send_request || (status_power == STATUS_POWER_PPS && millis() - time_PPS_request > t_PPSRequest))
  {
    wait_ps_rdy = 1;
    send_request = 0;
    time_PPS_request = millis();

    uint16_t header;
    uint32_t obj[7];
    /* Send request if option updated or regularly in PPS mode to keep power alive */
    PD_protocol_create_request(&protocol, &header, obj);
    // status_log_event(STATUS_LOG_MSG_TX, obj);
    time_wait_ps_rdy = millis();
    FUSB302_tx_sop(&FUSB302, header, obj);
    ESP_LOGI("FUSB302", "Send request");
  }
}

void handle_FUSB302_event(FUSB302_event_t events)
{
  if (events & FUSB302_EVENT_DETACHED)
  {
    PD_protocol_reset(&protocol);
    permitted_current = 500; // default usb
    ESP_LOGI("FUSB302", "event PD detached");
    return;
  }
  if (events & FUSB302_EVENT_ATTACHED)
  {
    uint8_t cc1 = 0, cc2 = 0, cc = 0;
    FUSB302_get_cc(&FUSB302, &cc1, &cc2);
    ESP_LOGI("FUSB302", "event PD attached, CC1 %d, CC2 %d", cc1, cc2);

    PD_protocol_reset(&protocol);
    if (cc1 && cc2 == 0)
    {
      cc = cc1;
    }
    else if (cc2 && cc1 == 0)
    {
      cc = cc2;
    }

    /* TODO: handle no cc detected error */
    if (cc > 1)
    {
      wait_src_cap = 1;
    }
    else
    {
      set_default_power();
    }
  }
  if (events & FUSB302_EVENT_RX_SOP)
  {
    PD_protocol_event_t protocol_event = 0;
    uint16_t header;
    uint32_t obj[7];

    FUSB302_get_message(&FUSB302, &header, obj);
    ESP_LOGI("FUSB302", "event RX_SOP, header 0x%X, num of obj %d, header type %d", header, (header >> 12) & 0x7, (header >> 0) & 0x1F);

    PD_protocol_handle_msg(&protocol, header, obj, &protocol_event);
    if (protocol_event)
    {
      handle_protocol_event(protocol_event);
    }
  }
  if (events & FUSB302_EVENT_GOOD_CRC_SENT)
  {
    ESP_LOGI("FUSB302", "event GOOD_CRC_SENT");

    uint16_t header;
    uint32_t obj[7];
    delay(2); /* Delay respond in case there are retry messages */
    if (PD_protocol_respond(&protocol, &header, obj))
    {
      // status_log_event(STATUS_LOG_MSG_TX, obj);
      FUSB302_tx_sop(&FUSB302, header, obj);
    }
  }
}

void handle_protocol_event(PD_protocol_event_t events)
{
  if (events & PD_PROTOCOL_EVENT_SRC_CAP)
  {
    wait_src_cap = 0;
    get_src_cap_retry_count = 0;
    wait_ps_rdy = 1;
    time_wait_ps_rdy = millis();
    ESP_LOGI("FUSB302", "PD_PROTOCOL_EVENT_SRC_CAP");
  }
  if (events & PD_PROTOCOL_EVENT_REJECT)
  {
    if (wait_ps_rdy)
    {
      wait_ps_rdy = 0;
      // status_log_event(STATUS_LOG_POWER_REJECT);
    }
    ESP_LOGI("FUSB302", "PD_PROTOCOL_EVENT_REJECT");
  }
  if (events & PD_PROTOCOL_EVENT_PS_RDY)
  {
    PD_power_info_t p;
    uint8_t i, selected_power = PD_protocol_get_selected_power(&protocol);
    PD_protocol_get_power_info(&protocol, selected_power, &p);
    wait_ps_rdy = 0;
    if (p.type == PD_PDO_TYPE_AUGMENTED_PDO)
    {
      // PPS mode
      FUSB302_set_vbus_sense(&FUSB302, 0);
      if (PPS_voltage_next)
      {
        // Two stage startup for PPS voltage < 5V
        PD_protocol_set_PPS(&protocol, PPS_voltage_next, PPS_current_next, false);
        PPS_voltage_next = 0;
        send_request = 1;
        // status_log_event(STATUS_LOG_POWER_PPS_STARTUP);
        ESP_LOGI("FUSB302", "PD_PROTOCOL_EVENT_PS_RDY PPS_STARTUP");
      }
      else
      {
        time_PPS_request = millis();
        status_power_ready(STATUS_POWER_PPS, PD_protocol_get_PPS_voltage(&protocol), PD_protocol_get_PPS_current(&protocol));
        // status_log_event(STATUS_LOG_POWER_READY);
        ESP_LOGI("FUSB302", "PD_PROTOCOL_EVENT_PS_RDY POWER_READY");
      }
    }
    else
    {
      FUSB302_set_vbus_sense(&FUSB302, 1);
      status_power_ready(STATUS_POWER_TYP, p.max_v, p.max_i);
      // status_log_event(STATUS_LOG_POWER_READY);
      ESP_LOGI("FUSB302", "PD_PROTOCOL_EVENT_PS_RDY set default");
    }
  }
}

void set_default_power(void)
{
  status_power_ready(STATUS_POWER_TYP, PD_V(5), PD_A(1));
  // status_log_event(STATUS_LOG_POWER_READY);
}

void status_power_ready(status_power_t status, uint16_t voltage, uint16_t current)
{
  ready_voltage = voltage;
  ready_current = current;
  status_power = status;

  switch (status)
  {
  case STATUS_POWER_TYP:
    ESP_LOGI("FUSB302", "PD attached at normal mode with voltage: %d mV, current: %d mA", ready_voltage * 50, ready_current * 10);
    permitted_current = ready_current * 10;
    break;
  case STATUS_POWER_PPS:
    ESP_LOGI("FUSB302", "PD attached at PPS mode with voltage: %d mV, current: %d mA", ready_voltage * 20, ready_current * 50);
    permitted_current = ready_current * 50;
    break;
  default:
    break;
  }
}

FUSB302_ret_t FUSB302_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t count)
{
  Wire.beginTransmission(dev_addr);
  Wire.write(reg_addr);
  Wire.endTransmission();
  Wire.requestFrom(dev_addr, count);
  while (Wire.available() && count > 0)
  {
    *data++ = Wire.read();
    count--;
  }

  ESP_EARLY_LOGV("FUSB302", "Read addr: %X, res count %d, value %X", reg_addr, count, data[0]);

  return count == 0 ? FUSB302_SUCCESS : FUSB302_ERR_READ_DEVICE;
}

FUSB302_ret_t FUSB302_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t count)
{
  Wire.beginTransmission(dev_addr);
  Wire.write(reg_addr);
  while (count > 0)
  {
    Wire.write(*data++);
    count--;
  }
  Wire.endTransmission();

  return FUSB302_SUCCESS;
}

FUSB302_ret_t FUSB302_delay_ms(uint32_t t)
{
  delay(t);

  return FUSB302_SUCCESS;
}
