#include <Arduino.h>
#include "constants.h"

// Forward declaration
class StateMachine;

// Button press types
enum ButtonPressType {
    SHORT_PRESS,
    LONG_PRESS,
    NO_PRESS
};

void setupButton();
ButtonPressType checkButtonPress();
void handleButtonEvents(StateMachine& stateMachine);

