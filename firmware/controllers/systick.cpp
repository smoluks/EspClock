#include "h/systick.hpp"

//get current time in uS
inline timestamp_uS_t GetTimestamp()
{
    return esp_timer_get_time();
}

//get time after current, in uS
inline timestamp_uS_t GetTimestamp(uint64_t after_current_in_us)
{
    return esp_timer_get_time() + after_current_in_us;
}

//check munis flag, but in unsigned logic
inline bool IsTimeout(timestamp_uS_t timestamp)
{
    return (timestamp - (uint64_t)esp_timer_get_time()) & 0x8000000000000000;
}