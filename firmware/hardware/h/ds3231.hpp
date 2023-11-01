void DS3231Init();
void DS3231SetTime(uint32_t epochTime);
inline void DS3231RefreshTime();
inline uint8_t DS3231GetMinute();
inline uint8_t DS3231GetHour();