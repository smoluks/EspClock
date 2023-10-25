#define LIGHT_ANALOG_PIN 34

void light_loop()
{
    uint16_t raw = analogRead(LIGHT_ANALOG_PIN);

    uint16_t value = 0;
    if(raw > 1200)
        value = (raw - 1200) / 64;

    hub75_set_brigthness(value > 255 ? 255 : value);

    //ESP_LOGI("light", "value: %d", value);
}