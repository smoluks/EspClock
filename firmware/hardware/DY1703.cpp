/**
 * Wrapper for DY1703A in UART mode
 * Thanks SnijderC https://github.com/SnijderC/dyplayer for good description 
 */

#include "../h/settings.hpp"
#include "../h/hardware.hpp"
#include "../managers/h/errorManager.hpp"
#include "h/DY1703.hpp"

enum audio_source_e
{
    AUDIO_SOURCE_USB = 0,
    AUDIO_SOURCE_SD = 1,
    AUDIO_SOURCE_FLASH = 2,
    AUDIO_SOURCE_SPI = 4,
    AUDIO_SOURCE_ERROR = 0XFF
};

extern settings_t settings;

enum audio_source_e DY1703GetAudioSource();
void sendCommandWithoutAnswer(uint8_t command[], uint8_t size);
uint8_t sendCommandWithAnswer(const uint8_t command[], uint8_t command_size, uint8_t answer[], uint8_t answer_size);
void calculateChecksum(uint8_t buffer[], uint8_t size);

void TakeUart();
void ReturnUart();

void DY1703Init()
{
    ESP_LOGI("DY1703", "audio init started");

    enum audio_source_e source = DY1703GetAudioSource();
    switch (source)
    {
    case AUDIO_SOURCE_USB:
        ESP_LOGI("DY1703", "Source: USB falsh drive");
        break;
    case AUDIO_SOURCE_SD:
        ESP_LOGI("DY1703", "Source: SD card");
        break;
    case AUDIO_SOURCE_FLASH:
        ESP_LOGI("DY1703", "Source: flash"); //IDK what it is, looks like old spi value
        break;
    case AUDIO_SOURCE_SPI:
        ESP_LOGI("DY1703", "Source: SPI memory");
        break;
    default:
        setError(ERROR_AUDIO_CHIP_NOT_FOUND_OR_HAVE_NO_SOURCE);
        ESP_LOGE("DY1703", "audio chip not found or have no source");
        return;
    }

    DY1703Stop();

    ESP_LOGI("DY1703", "audio init finished");
}

void DY1703SetVolume(uint8_t volume)
{
    if (volume > 30)
        volume = 30;

    uint8_t command[5] = {0xAA, 0x13, 0x01, volume};
    calculateChecksum(command, 4);

    sendCommandWithoutAnswer(command, sizeof(command));
}

void DY1703Play()
{
    uint8_t command[4] = {0xAA, 0x02, 0x00, 0xAC};
    sendCommandWithoutAnswer(command, sizeof(command));
}

void DY1703Stop()
{
    uint8_t command[4] = {0xAA, 0x04, 0x00, 0xAE};
    sendCommandWithoutAnswer(command, sizeof(command));
}

enum audio_source_e DY1703GetAudioSource()
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
        return AUDIO_SOURCE_ERROR;
    }
}

void sendCommandWithoutAnswer(uint8_t command[], uint8_t size)
{
    TakeUart();

    DY1703Serial.write(command, size);

    ReturnUart();
}

uint8_t sendCommandWithAnswer(const uint8_t command[], uint8_t command_size, uint8_t answer[], uint8_t answer_size)
{
    TakeUart();

    uint8_t count;
    uint8_t repeat_count = 0;
    do
    {
        DY1703Serial.write(command, command_size);
        count = DY1703Serial.readBytes(answer, answer_size);
    } while (count != answer_size && repeat_count++ < COMMAND_REPEAT_COUNT);

    ReturnUart();

    return count;
}

void calculateChecksum(uint8_t buffer[], uint8_t size)
{
    uint8_t sum = 0;
    for (uint8_t i = 0; i < size; i++)
    {
        sum += buffer[i];
    }

    buffer[size] = sum;
}

void TakeUart()
{
    DY1703Serial.flush(true);
    Serial.setDebugOutput(false);

    DY1703Serial.updateBaudRate(9600);
    // Make make attention - core may be much faster than ABP bus exchange
    // Core has clock up to 240MHz and APB bus - only 80MHz, so therefore be only 40kk transactions per second
    // So we should wait for confirmation
    uint32_t baudRate;
    do
    {
        delay(1);
        baudRate = DY1703Serial.baudRate();
    } while (baudRate != 9600 - 1 && baudRate != 9600 && baudRate != 9600 + 1);
}

void ReturnUart()
{
    DY1703Serial.flush(true);

#if (DY1703Serial == DebugSerial)
    DY1703Serial.updateBaudRate(DEBUG_SPEED);
    uint32_t baudRate;
    do
    {
        delay(1);
        baudRate = DY1703Serial.baudRate();
    } while (baudRate != DEBUG_SPEED - 1 && baudRate != DEBUG_SPEED && baudRate != DEBUG_SPEED + 1);

    Serial.setDebugOutput(true);
#endif    
}