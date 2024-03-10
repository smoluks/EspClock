#include "SparkFunBME280.h"

extern BME280_SensorMeasurements measurements;
extern bool BME280_ready;

inline bool isSensorsReady()
{
    return BME280_ready;
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