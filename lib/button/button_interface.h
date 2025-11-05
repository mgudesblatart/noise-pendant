#include <Arduino.h>

#define BUTTON_PIN GPIO_NUM_3
#define LONG_PRESS_DURATION 2000  // 2 seconds in milliseconds

// Button press types
enum ButtonPressType {
    SHORT_PRESS,
    LONG_PRESS,
    NO_PRESS
};

void setupButton();
ButtonPressType checkButtonPress();
void handleButtonEvents();

