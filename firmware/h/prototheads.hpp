#include "libs/pt-1.4/pt.h"

// screen handlers
typedef enum pt_return_e
{
    pt_waiting,
    pt_yieled,
    pt_exited,
    pt_ended
} pt_return;