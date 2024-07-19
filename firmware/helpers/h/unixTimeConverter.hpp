#pragma once

#include "../../hardware/h/ds3231.hpp"

bool TryConvertUnixTimeToDateTime(uint32_t unixTime, DateTime* result);