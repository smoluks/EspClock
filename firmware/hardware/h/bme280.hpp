#define SEALEVELPRESSURE_HPA (1013.25)

void BME280Init();
float BME280GetTemperature();
float BME280GetPressure();
float BME280GetAltitude();
float BME280GetHumidity();