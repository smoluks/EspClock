#include "h/errorManager.hpp"

static sys_error_t errors[MAX_ERRORS_COUNT];
static uint8_t error_count = 0;

void setError(sys_error_t error)
{
    if(error == ERROR_NOERROR)
        return;

    if(error_count == MAX_ERRORS_COUNT)
        return;

    for(uint8_t i = 0; i < error_count; i++)
    {
        if(errors[i] == error)
            return;
    }

    errors[error_count++] = error;
}

inline bool isErrors()
{
    return error_count;
}

inline sys_error_t* getErrors()
{
    return errors;
}

inline uint8_t getErrorsCount()
{
    return error_count;
}