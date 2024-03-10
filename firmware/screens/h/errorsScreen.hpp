#pragma once

#include "screenCommon.hpp"

#define ERROR_SCREEN_REFRESH_PERIOD 1000

void errorScreenInit();
screen_action_e errorScreenLoop();