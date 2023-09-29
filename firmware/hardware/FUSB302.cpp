#include "PD_UFP.h"

#define FUSB302_INT_PIN   35

PD_UFP_c PD_UFP;

void FUSB302_init()
{
    PD_UFP.init(FUSB302_INT_PIN, PD_POWER_OPTION_MAX_12V);

    ESP_LOGI("FUSB302", "Voltage: %d, in 50mV units, 20mV(PPS)", PD_UFP.get_voltage()); 
    ESP_LOGI("FUSB302", "Current: %d, in 10mA units, 50mA(PPS)", PD_UFP.get_current()); 
}

void FUSB302_loop()
{
  PD_UFP.run();
  if (PD_UFP.is_power_ready())
  {
    Serial.write("PD supply connected\n");
  }
  else
  {
    Serial.write("No PD supply available\n");
  }
}