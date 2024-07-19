//get current time
inline uint32_t GetTimestamp()
{
    return esp_timer_get_time();
}

//get time after current
inline uint32_t GetTimestamp(uint32_t after_current_in_ms)
{
    return esp_timer_get_time() + after_current_in_ms * 1000;
}

//check munis flag, but in unsigned logic
inline bool IsTimeout(uint32_t timestamp)
{
    return (timestamp - esp_timer_get_time()) & 0x80000000;
}