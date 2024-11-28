#pragma once

typedef void (* float_callback)(float data);
typedef void (* int8_callback)(int8_t data);
typedef void (* bool_callback)(bool data);

void BTHomeInit(float_callback temperatureCallback,
                float_callback humidityCallback,
                int8_callback rssiCallback,
                int8_callback batteryCallback,
                bool_callback connectCallback);
void BTHomeLoop();