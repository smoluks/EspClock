#include "PD_UFP.h"

#define FUSB302_INT_PIN   35

PD_UFP_c PD_UFP;
//PD_UPD_UFP_log_c PD_UFP;

void IRAM_ATTR FUSB302Interrupt()
{
  ESP_LOGI("FUSB302", "FUSB302 Int"); 
  PD_UFP.run();
}

void FUSB302Init()
{    
  ESP_LOGI("FUSB302", "FUSB302 init started");

    PD_UFP.init(FUSB302_INT_PIN, PD_POWER_OPTION_MAX_12V);

    //attachInterrupt(FUSB302_INT_PIN, FUSB302Interrupt, FALLING);
    
    uint32_t timestamp = millis();
    do {
      PD_UFP.run();
    }
    while (PD_UFP.get_ps_status() == STATUS_POWER_NA && millis() - timestamp < 1000);

    Hub75MoveLoadingBar();
    
    ESP_LOGI("FUSB302", "PD init finished with status: %d at %d ms", PD_UFP.get_ps_status(), millis() - timestamp);    
}

status_power_t old_status = STATUS_POWER_NA;
void FUSB302_loop()
{
  PD_UFP.run();

  if(PD_UFP.get_ps_status() != old_status)
  {
    old_status = PD_UFP.get_ps_status();
    switch (old_status)
    {
    case STATUS_POWER_TYP:
      ESP_LOGI("FUSB302", "PD attached at normal mode with voltage: %d mV, current: %d mA", PD_UFP.get_voltage() * 50, PD_UFP.get_current() * 10); 
      break;
    case STATUS_POWER_PPS:
      ESP_LOGI("FUSB302", "PD attached at PPS mode with voltage: %d mV, current: %d mA", PD_UFP.get_voltage() * 20, PD_UFP.get_current() * 50); 
      break;
    default:
      ESP_LOGI("FUSB302", "PD detached"); 
      break;
    }
  }
}