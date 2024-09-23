#pragma once

#include "../../hardware/h/ds3231.hpp"

inline bool IsTimePresent();
inline DateTime GetCurrentTime();
inline void SetCurrentTime(DateTime time);
void UpdateTimeFromNTP();