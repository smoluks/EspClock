#pragma once

float ExternalTemperature;
bool ExternalTemperaturePresent;
float ExternalHumidity;
bool ExternalHumidityPresent;
int8_t ExternalRSSI;
bool ExternalRSSIPresent;
uint8_t ExternalBatteryLevel;
bool ExternalBatteryLevelPresent;
bool ExternalConnected;

void ExternalSensorsInit();
void ExternalSensorsLoop();