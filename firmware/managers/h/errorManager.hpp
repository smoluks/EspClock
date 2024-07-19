#pragma once

typedef enum sys_error_e
{
    ERROR_NOERROR = 0,
    ERROR_BME280_NOT_FOUND = 1,
    ERROR_AUDIO_CHIP_NOT_FOUND_OR_HAVE_NO_SOURCE = 2,
    ERROR_DS3231_NOT_FOUND = 3,
    ERROR_FUSB302_NOT_FOUND = 4,
    
} sys_error_t;

#define MAX_ERRORS_COUNT 16

void setError(sys_error_t error);
bool isErrors();
sys_error_t* getErrors();
uint8_t getErrorsCount();