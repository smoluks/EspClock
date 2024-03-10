#include "SparkFunBME280.h"
#include "../managers/h/errorManager.hpp"

BME280_SensorMeasurements measurements;
bool BME280_ready;

BME280 _mySensor;

void BME280Init()
{
    ESP_LOGI("BM*280", "BM*280 init started");

    uint8_t id = _mySensor.begin();
    switch (id)
    {
    case 0x58:
        ESP_LOGI("BM*280", "BMP280 found");
        break;
    case 0x60:
        ESP_LOGI("BM*280", "BME280 found");
        break;
    default:
        setError(ERROR_BME280_NOT_FOUND);
        ESP_LOGE("BM*280", "Unknown chip, id = 0x%X", id);
    }    

    _mySensor.setReferencePressure(101200);

    ESP_LOGI("BM*280", "BM*280 init finished");
}

void BME280Loop()
{
    if (_mySensor.isMeasuring())
        return;

    _mySensor.readAllMeasurements(&measurements);

    BME280_ready = true;
}
