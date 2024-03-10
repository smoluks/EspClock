#pragma once

#include "../../h/errors.hpp"

#define MAX_ERRORS_COUNT 16

void setError(sys_error_t error);
bool isErrors();
sys_error_t* getErrors();
uint8_t getErrorsCount();