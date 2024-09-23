extern void (*T6703ValueHandler)(uint16_t value);

static bool co2ready = false;
static uint16_t co2value;
static void co2ChangedHandler(uint16_t value)
{
    co2value = value;
    co2ready = true;
}

inline void CO2Init()
{
    T6703ValueHandler = co2ChangedHandler;
}

inline bool IsCO2Present()
{
    return co2ready;
}

inline uint16_t GetCO2Value()
{
    return co2value;
}