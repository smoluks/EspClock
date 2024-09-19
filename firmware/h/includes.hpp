#pragma once

//-----libs-----
#include "../libs/FUSB302_PD_UFP_sink/src/FUSB302_UFP.cpp"
#include "../libs/FUSB302_PD_UFP_sink/src/PD_UFP_Protocol.cpp"
#include "../libs/pt-1.4/pt.h"

//-----source files-----
//IDK how to make it automatically
//because Arduino IDE doesn't support auto link files in source folders
//if anybody has any ideas, please dm me
//controllers
#include "../controllers/envSensorsController.cpp"
#include "../controllers/clockController.cpp"
#include "../controllers/co2Controller.cpp"
#include "../controllers/systickController.cpp"
#include "../controllers/powerController.cpp"
//#include "../controllers/webController.cpp"
//hardware
#include "../hardware/bme280.cpp"
#include "../hardware/current.cpp"
#include "../hardware/ds3231.cpp"
#include "../hardware/DY1703.cpp"
#include "../hardware/FUSB302.cpp"
#include "../hardware/hub75.cpp"
#include "../hardware/i2c.cpp"
#include "../hardware/light.cpp"
#include "../hardware/T6703.cpp"
#include "../hardware/touch.cpp"
#include "../hardware/voltage.cpp"
#include "../hardware/WiFi.cpp"
//helpers
#include "../helpers/unixTimeConverter.cpp"
//managers
#include "../managers/errorManager.cpp"
#include "../managers/screenManager.cpp"
#include "../managers/settingsManager.cpp"
//screens
#include "../screens/clockScreen.cpp"
#include "../screens/effectsScreen.cpp"
#include "../screens/errorsScreen.cpp"
#include "../screens/sensorsScreen.cpp"
//tasks
#include "../tasks/i2cTask.cpp"

