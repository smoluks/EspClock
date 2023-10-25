#include "PD_UFP.h"

#define FUSB302_INT_PIN   35

PD_UFP_c PD_UFP;
//PD_UPD_UFP_log_c PD_UFP;

void FUSB302_init()
{
    PD_UFP.init(FUSB302_INT_PIN, PD_POWER_OPTION_MAX_12V);
    
    uint32_t timestamp = millis();
    do {
      PD_UFP.run();
      //PD_UFP.print_status(Serial);
    }
    while (PD_UFP.get_ps_status() == STATUS_POWER_NA && millis() - timestamp < 10000);

    ESP_LOGI("FUSB302", "PD supply procedure finished at: %d ms", millis() - timestamp);
    ESP_LOGI("FUSB302", "PD supply status: %d", PD_UFP.get_ps_status());
    switch (PD_UFP.get_ps_status())
    {
    case STATUS_POWER_TYP:
      ESP_LOGI("FUSB302", "Voltage: %d mV", PD_UFP.get_voltage() * 50); 
      ESP_LOGI("FUSB302", "Current: %d mA", PD_UFP.get_current() * 10); 
      break;
    case STATUS_POWER_PPS:
      ESP_LOGI("FUSB302", "Voltage: %d mV", PD_UFP.get_voltage() * 20); 
      ESP_LOGI("FUSB302", "Current: %d mA", PD_UFP.get_current() * 50); 
      break;
    default:
      break;
    }
}

void FUSB302_loop()
{
  PD_UFP.run();
  //PD_UFP.print_status(Serial);
}