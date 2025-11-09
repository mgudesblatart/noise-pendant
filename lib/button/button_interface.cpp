#include <U8g2lib.h>
#include <ezButton.h>
#include "button_interface.h"
#include "constants.h"
#include "thresholds.h"
#include "state_machine.h"
#include "power_management.h"

unsigned long pressedTime = 0;
unsigned long releasedTime = 0;
bool isPressing = false;
bool isLongDetected = false;

ezButton button(BUTTON_PIN);
void setupButton()
{
    button.setDebounceTime(BUTTON_DEBOUNCE_MS);

    // Enable GPIO wakeup for light sleep compatibility
    enableGPIOWakeup(BUTTON_PIN);

    Serial.println("Button setup: Pure polling mode");
}


ButtonPressType checkButtonPress()
{
    // Serial.println("Checking button press...");

    ButtonPressType pressType = NO_PRESS;
    // Serial.print("Button state: ");
    // Serial.println(button.getStateRaw());
    if (button.isPressed())
    {
        // Serial.println("Button pressed");
        pressedTime = millis();
        isPressing = true;
        isLongDetected = false;
    }

    if (button.isReleased())
    {
        isPressing = false;
        isLongDetected = false;
        releasedTime = millis();

        long pressDuration = releasedTime - pressedTime;
        // Serial.println("Button released, duration: " + String(pressDuration) + " ms");

        if (pressDuration < BUTTON_LONG_PRESS_MS)
        {

            Serial.println("A short press is detected");
            pressType = SHORT_PRESS;
            return pressType;
        }
    }

    if (isPressing == true && isLongDetected == false)
    {
        long pressDuration = millis() - pressedTime;

        if (pressDuration > BUTTON_LONG_PRESS_MS)
        {
            Serial.println("A long press is detected");
            isLongDetected = true;
            pressType = LONG_PRESS;
        }
    }

    return pressType;
}

void handleButtonEvents(StateMachine &stateMachine)
{
    // Serial.println("Handling button event");
    button.loop();
    ButtonPressType pressType = checkButtonPress();
    if (pressType == NO_PRESS)
        return;
    switch (pressType)
    {
    case SHORT_PRESS:
        if (stateMachine.isAlarmActive())
        {
            // Acknowledge alarm
            stateMachine.acknowledgeAlarm();
            Serial.println("Alarm acknowledged by short press.");
            return;
        }
        if (!stateMachine.isInConfigMode())
        {
            stateMachine.enterConfigMode();
            return;
        }
        if (stateMachine.isInConfigMode())
        {
            stateMachine.cycleConfigMode();
            return;
        }
        break;
    case LONG_PRESS:
        if (stateMachine.isInConfigMode())
        {
            stateMachine.exitConfigMode(true);
            return;
        }
        break;
    default:
        break;
    }
}
