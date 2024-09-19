#include "SparkFunBME280.h"
#include "h/bme280.hpp"
#include "../managers/h/errorManager.hpp"

static const char *BME280_TAG = "BME280";

BME280_SensorMeasurements BME280Measurements;
bool BME280_results_ready;
uint8_t BME280_id;
static BME280 _mySensor;

void BME280Init()
{ 
    ESP_LOGI(BME280_TAG, "BM*280 init started");

    BME280_id = _mySensor.begin();
    switch (BME280_id)
    {
    case BMP280_ID:
        ESP_LOGI(BME280_TAG, "BMP280 found");
        break;
    case BME280_ID:
        ESP_LOGI(BME280_TAG, "BME280 found");
        break;
    default:
        setError(ERROR_BME280_NOT_FOUND);
        ESP_LOGE(BME280_TAG, "Unknown chip, id = 0x%X", BME280_id);
    }    

    _mySensor.setReferencePressure(101200);

    ESP_LOGI(BME280_TAG, "BM*280 init finished");
}

void BME280Loop()
{
    if (_mySensor.isMeasuring())
        return;

    _mySensor.readAllMeasurements(&BME280Measurements);

    BME280_results_ready = true;
}
