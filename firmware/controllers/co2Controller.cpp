extern uint16_t t6703_value;
extern bool t6703_ready;

inline bool IsCO2Present()
{
    return t6703_ready;
}

inline uint16_t GetCO2Value()
{
    return t6703_value;
}