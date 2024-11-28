#pragma once

#include "screenCommon.hpp"

#define EXTERNAL_SCREEN_SHOW 5000
#define EXTERNAL_SCREEN_REFRESH_PERIOD 1000

void externalScreenInit();
screen_action_e externalScreenLoop();