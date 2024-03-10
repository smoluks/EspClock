/**
 * Wrapper for DY1703A in UART mode
 */

#include "../h/settings.hpp"
#include "../managers/h/errorManager.hpp"
#include "h/DY1703.hpp"

enum audio_source_e
{
    audio_source_usb = 0,
    audio_source_SD = 1,
    audio_source_flash = 2,
    audio_source_SPI = 4,
    audio_source_error = 0xFF
};

extern settings_t settings;

enum audio_source_e checkDeviceOnline();
void sendCommandWithoutAnswer(uint8_t command[], uint8_t size);
uint8_t sendCommandWithAnswer(const uint8_t command[], uint8_t command_size, uint8_t answer[], uint8_t answer_size);
void calculateChecksum(uint8_t buffer[], uint8_t size);

void audioInit()
{
    ESP_LOGI("audio", "audio init started");

    enum audio_source_e source = checkDeviceOnline();
    if(source == audio_source_error)
    {
        setError(ERROR_AUDIO_CHIP_NOT_FOUND_OR_HAVE_NO_SOURCE);
        ESP_LOGE("audio", "audio chip not found or have no source");        
    } else
    {    
        stop();

        ESP_LOGI("audio", "audio source %X", source);
    }

    ESP_LOGI("audio", "audio init finished");
}

void setVolume(uint8_t volume)
{
    if(volume > 30)
        volume = 30;
  
    uint8_t command[5] = {0xAA, 0x13, 0x01, volume};
    calculateChecksum(command, 4);

    sendCommandWithoutAnswer(command, sizeof(command));
}

void play()
{  
    uint8_t command[4] = {0xAA, 0x02, 0x00, 0xAC};
    sendCommandWithoutAnswer(command, sizeof(command));    
}

void stop()
{  
    uint8_t command[4] = {0xAA, 0x04, 0x00, 0xAE};
    sendCommandWithoutAnswer(command, sizeof(command));    
}

enum audio_source_e checkDeviceOnline()
{
    uint8_t command[4] = {0xAA, 0x09, 0x00, 0xB3};
    uint8_t buffer[5];
    uint8_t count = sendCommandWithAnswer(command, sizeof(command), buffer, sizeof(buffer));
    if (count == 5)
    {
        return (enum audio_source_e)buffer[3];
    }
    else
    {
        return audio_source_error;
    }
}

void sendCommandWithoutAnswer(uint8_t command[], uint8_t size)
{
    Serial.flush();
    delay(2);
    Serial.updateBaudRate(9600);
    
    Serial.write(command, size);    

    Serial.flush();
    delay(2);
    Serial.updateBaudRate(115200);
}

uint8_t sendCommandWithAnswer(const uint8_t command[], uint8_t command_size, uint8_t answer[], uint8_t answer_size)
{
    Serial.flush();
    delay(2);
    Serial.updateBaudRate(9600);
    
    uint8_t count;
    uint8_t repeat_count = 0;
    do
    {        
        Serial.write(command, command_size);
        count = Serial.readBytes(answer, answer_size);
    } while (count != answer_size && repeat_count++ < COMMAND_REPEAT_COUNT);

    Serial.flush();
    delay(2);
    Serial.updateBaudRate(115200);

    return count;
}

void calculateChecksum(uint8_t buffer[], uint8_t size)
{
    uint8_t sum = 0;
    for(uint8_t i = 0; i < size; i++)
    {
        sum += buffer[i];
    }

    buffer[size] = sum;
}