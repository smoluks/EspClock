inline uint32_t getTimestamp()
{
    return millis();
}

inline uint32_t getTimestamp(uint32_t after_current_in_ms)
{
    return millis() + after_current_in_ms;
}

inline uint32_t getTimePassedFrom(uint32_t timestamp)
{
    return millis() - timestamp;
}

inline bool isTimeout(uint32_t timestamp)
{
    return (timestamp - millis()) & 0x80000000;
}