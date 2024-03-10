#pragma once

#include "PD_UFP.h"

void FUSB302Init();
void FUSB302Loop();
bool IsFUSB302PDPresent();
uint16_t FUSB302GetPermittedCurrent();
