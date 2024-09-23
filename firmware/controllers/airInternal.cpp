#include "SparkFunBME280.h"
#include "../hardware/h/bme280.hpp"

//#define SEALEVELPRESSURE_HPA (1013.25)

extern BME280_SensorMeasurements BME280Measurements;
extern bool BME280_results_ready;
extern uint8_t BME280_id;

inline bool IsInternalTemperaturePresent()
{
    return BME280_results_ready;
}

inline bool IsInternalPressurePresent()
{
    return BME280_results_ready;
}

inline bool IsInternalHumidityPresent()
{
    return BME280_id == BME280_ID && BME280_results_ready;
}

inline float GetInternalTemperature()
{
    return BME280Measurements.temperature;
}

inline float GetInternalPressure()
{
    return BME280Measurements.pressure / 100;
}

// float BME280GetAltitude()
//{
//     return bme.readAltitude(SEALEVELPRESSURE_HPA);
// }

inline float GetInternalHumidity()
{
    return BME280Measurements.humidity;
}