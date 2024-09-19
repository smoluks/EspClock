#pragma once

#define COMMAND_REPEAT_COUNT 3

enum audio_source_e
{
    AUDIO_SOURCE_USB = 0,
    AUDIO_SOURCE_SD = 1,
    AUDIO_SOURCE_FLASH = 2,
    AUDIO_SOURCE_SPI = 4,
    AUDIO_SOURCE_ERROR = 0XFF
};

void DY1703Init();
void DY1703SetVolume(uint8_t volume);
void DY1703Stop();
void DY1703Play();