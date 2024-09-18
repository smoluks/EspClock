#pragma once

#include "screenCommon.hpp"

#define SENSORS_SCREEN_SHOW_TIME 5000
#define SENSORS_SCREEN_REFRESH_PERIOD 200

void sensorsScreenInit();
screen_action_e sensorsScreenLoop();