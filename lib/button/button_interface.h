#include <Arduino.h>

// Forward declaration
class StateMachine;

#define BUTTON_PIN GPIO_NUM_3
#define LONG_PRESS_DURATION 2000  // 2 seconds in milliseconds
#define DEBOUNCE_DELAY 50         // 50ms debounce

// Button press types
enum ButtonPressType {
    SHORT_PRESS,
    LONG_PRESS,
    NO_PRESS
};

void setupButton();
ButtonPressType checkButtonPress();
void handleButtonEvents(StateMachine& stateMachine);

