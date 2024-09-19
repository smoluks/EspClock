/**
 * Wrapper for DY1703A in UART mode
 * Thanks SnijderC https://github.com/SnijderC/dyplayer for good description 
 */

#include "../h/settings.hpp"
#include "../h/hardware.hpp"
#include "../managers/h/errorManager.hpp"
#include "h/DY1703.hpp"

static const char *DY1703_TAG = "DY1703";

static enum audio_source_e DY1703GetAudioSource();
static void sendCommandWithoutAnswer(const uint8_t command[], uint8_t size, uint8_t checksum);
static uint8_t sendCommandWithAnswer(const uint8_t command[], uint8_t command_size, uint8_t checksum, uint8_t answer[], uint8_t answer_size);
static uint8_t calculateChecksum(uint8_t buffer[], uint8_t size);
static void takeUart();
static void returnUart();

extern settings_t settings;

void DY1703Init()
{
    ESP_LOGI(DY1703_TAG, "audio init started");

    enum audio_source_e source = DY1703GetAudioSource();
    switch (source)
    {
    case AUDIO_SOURCE_USB:
        ESP_LOGI(DY1703_TAG, "Source: USB flash drive");
        break;
    case AUDIO_SOURCE_SD:
        ESP_LOGI(DY1703_TAG, "Source: SD card");
        break;
    case AUDIO_SOURCE_FLASH:
        ESP_LOGI(DY1703_TAG, "Source: flash"); //IDK what it is, looks like old spi value
        break;
    case AUDIO_SOURCE_SPI:
        ESP_LOGI(DY1703_TAG, "Source: SPI memory");
        break;
    default:
        setError(ERROR_AUDIO_CHIP_NOT_FOUND_OR_HAVE_NO_SOURCE);
        ESP_LOGE(DY1703_TAG, "audio chip not found or have no source");
        return;
    }

    DY1703Stop();

    ESP_LOGI(DY1703_TAG, "audio init finished");
}

void DY1703SetVolume(uint8_t volume)
{
    if (volume > 30)
        volume = 30;

    static uint8_t command[4] = {0xAA, 0x13, 0x01, volume};

    sendCommandWithoutAnswer(command, sizeof(command), calculateChecksum(command, 4));
}

void DY1703Play()
{
    static const uint8_t command[3] = {0xAA, 0x02, 0x00};
    sendCommandWithoutAnswer(command, sizeof(command), 0xAC);
}

void DY1703Stop()
{
    static const uint8_t command[3] = {0xAA, 0x04, 0x00};
    sendCommandWithoutAnswer(command, sizeof(command), 0xAE);
}

enum audio_source_e DY1703GetAudioSource()
{
    static const uint8_t command[3] = {0xAA, 0x09, 0x00};
    uint8_t buffer[5];
    uint8_t count = sendCommandWithAnswer(command, sizeof(command), 0xB3, buffer, sizeof(buffer));
    if (count == 5)
    {
        return (enum audio_source_e)buffer[3];
    }
    else
    {
        ESP_LOGE(DY1703_TAG, "GetAudioSource answer length %d", count);
        return AUDIO_SOURCE_ERROR;
    }
}

void sendCommandWithoutAnswer(const uint8_t command[], uint8_t size, uint8_t checksum)
{
    takeUart();

    DY1703Serial.write(command, size);
    DY1703Serial.write(checksum);

    returnUart();
}

uint8_t sendCommandWithAnswer(const uint8_t command[], uint8_t command_size, uint8_t checksum, uint8_t answer[], uint8_t answer_size)
{
    takeUart();

    uint8_t count;
    uint8_t repeat_count = 0;
    while (true)
    {
        DY1703Serial.write(command, command_size);
        DY1703Serial.write(checksum);
        count = DY1703Serial.readBytes(answer, answer_size);

        if (count == answer_size || repeat_count++ > COMMAND_REPEAT_COUNT)
        {
            break;
        }

        delay(200);
    }

    returnUart();

    return count;
}

uint8_t calculateChecksum(const uint8_t buffer[], uint8_t size)
{
    uint8_t sum = 0;
    for (uint8_t i = 0; i < size; i++)
    {
        sum += buffer[i];
    }

    return sum;
}

void takeUart()
{
    DY1703Serial.flush(true);
    Serial.setDebugOutput(false);
    DY1703Serial.setRxTimeout(500);
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

void returnUart()
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