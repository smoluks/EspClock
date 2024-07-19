#include "SparkFunBME280.h"
#include "../hardware/h/bme280.hpp"

//#define SEALEVELPRESSURE_HPA (1013.25)

extern BME280_SensorMeasurements measurements;
extern bool BME280_results_ready;
extern uint8_t BME280_id;

inline bool IsTemperaturePresent()
{
    return BME280_results_ready;
}

inline bool IsPressurePresent()
{
    return BME280_results_ready;
}

inline bool IsHumidityPresent()
{
    return BME280_id == BME280_ID && BME280_results_ready;
}

inline float GetTemperature()
{
    return measurements.temperature;
}

inline float GetPressure()
{
    return measurements.pressure / 100;
}

// float BME280GetAltitude()
//{
//     return bme.readAltitude(SEALEVELPRESSURE_HPA);
// }

inline float GetHumidity()
{
    return measurements.humidity;
}