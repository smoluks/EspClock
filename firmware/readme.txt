-----Project structure-----
h
   includes.hpp   all source(cpp) files includes for arduino framework compatibility
   hardware.hpp pins and other hardware-specific things defines
   settings.hpp   user settings structure
-hardware        provides hardware-dependent API - drivers, etc.
-controllers     provides hardware-independent API. Please note hardware and controllers code may be executed in different OS tasks and different cores, thread sychronization may be needed.
   For ex. for all I2C sensors hardware code calls in I2C RTOS task and controllers code in default task.
-helpers         methods, containing only logic, without domains. Must be stateless
-libs            libs which can be connected as arduino libs. Copypasted without any changes in files, but unused was deleted
-managers        non-hardware logic
-screens         code for display drawing
-tasks           RTOS tasks

-----Code style-----
Because arduino, we don't have file scope, so static global attribute not working
private variable, functions - camelCase
public variable, functions - PascalCase