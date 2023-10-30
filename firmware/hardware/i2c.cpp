#include "uRTCLib.h"

void I2CInit()
{
    if (!URTCLIB_WIRE.begin(21, 22, 400000))
    {
        Serial.print("I2C init error");
    }
}