#include "../screens/h/sensorsScreen.hpp"
#include "../screens/h/clockScreen.hpp"
#include "../screens/h/errorsScreen.hpp"
#include "../screens/h/effectsScreen.hpp"
#include "../screens/h/screenCommon.hpp"
#include "../managers/h/errorManager.hpp"
#include "h/screenManager.hpp"

// screen handlers
enum screens_e
{
    no = 0,
    timeScreen,
    sensorsScreen,
    errorScreen,
    effectsScreen
} current_screen;

void (*initFns[])() = {NULL, &clockScreenInit, &sensorsScreenInit, &errorScreenInit, &effectsScreenInit};
screen_action_t (*loopFns[])() = {NULL, &clockScreenLoop, &sensorsScreenLoop, &errorScreenLoop, &effectsScreenLoop};

void moveToScreen(enum screens_e screen);
void checkChangeConditions();

// Loop manager
screen_action_t (*loopF)() = NULL;
void screenManagerLoop()
{
    checkChangeConditions();

    // process current screen
    if (loopF != NULL)
    {
        screen_action_t result = (*loopF)();
        switch (result)
        {
        case SCREEN_ACTION_GO_TO_DEFAULT:
            moveToScreen(timeScreen);
            break;
        case SCREEN_ACTION_GO_TO_NEXT:
            switch (current_screen)
            {
            case timeScreen:
                moveToScreen(sensorsScreen);
                break;
            case sensorsScreen:
                moveToScreen(effectsScreen);
                break;
            default:
                break;
            }
            
            break;
        }
    }
}

void checkChangeConditions()
{
    // check logic condition for changing screen
    if (current_screen == no)
        moveToScreen(timeScreen);

    if (isErrors())
        moveToScreen(errorScreen);

    /*if(!IsFUSB302PDPresent())
{
  HUB75PCModeScreen();
  for (;;);
}*/
}

void moveToScreen(enum screens_e screen)
{
    if (current_screen == screen)
        return;

    current_screen = screen;

    loopF = NULL;

    if ((initFns[screen]) != NULL)
        (*(initFns[screen]))();

    loopF = loopFns[screen];
}
