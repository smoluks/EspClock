extern uint16_t t6703_value;
extern bool t6703_ready;

inline bool isCO2Ready()
{
    return t6703_ready;
}

inline uint16_t CO2Value()
{
    return t6703_value;
}