#include "h/errorManager.hpp"
#include "../h/errors.hpp"

sys_error_t _errors[MAX_ERRORS_COUNT];
uint8_t error_count = 0;

void setError(sys_error_t error)
{
    if(error == ERROR_NOERROR)
        return;

    if(error_count == MAX_ERRORS_COUNT)
        return;

    for(uint8_t i = 0; i < error_count; i++)
    {
        if(_errors[i] == error)
            return;
    }

    _errors[error_count++] = error;
}

inline bool isErrors()
{
    return error_count;
}

inline sys_error_t* getErrors()
{
    return _errors;
}

inline uint8_t getErrorsCount()
{
    return error_count;
}