#ifndef CLOCK_HPP
#define CLOCK_HPP

#include <libs/pt-1.4/pt.h>
#include "../../hardware/h/ds3231.hpp"

void ClockLoop();
void TimeChangedHandler(DateTime value);

inline bool IsTimePresent();
inline DateTime GetCurrentTime();
inline void SetCurrentTime(DateTime time);
void UpdateTimeFromNTP();

#endif // CLOCK_HPP