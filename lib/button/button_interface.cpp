#include <U8g2lib.h>
#include "button_interface.h"
#include "thresholds.h"
#include "state_machine.h"

extern StateMachine stateMachine;

volatile bool buttonPressed = false;
volatile unsigned long buttonPressStartTime = 0;
volatile bool buttonEventProcessed = true;

volatile unsigned long lastInterruptTime = 0;

void IRAM_ATTR buttonISR()
{
    unsigned long now = millis();
    if (now - lastInterruptTime < DEBOUNCE_DURATION)
    {
        return; // Ignore bounce
    }
    lastInterruptTime = now;

    bool currentState = digitalRead(BUTTON_PIN);
    if (currentState == LOW)
    { // Button pressed (active low)
        if (!buttonPressed)
        { // First press
            buttonPressStartTime = now;
            buttonPressed = true;
            buttonEventProcessed = false;
        }
    }
    else
    { // Button released
        if (buttonPressed)
        {
            buttonPressed = false;
        }
    }
}

void setupButton()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, CHANGE);
}

ButtonPressType checkButtonPress()
{
    if (buttonEventProcessed)
    {
        return NO_PRESS;
    }

    unsigned long now = millis();

    // If button is still pressed, check for long press
    if (buttonPressed && (now - buttonPressStartTime >= LONG_PRESS_DURATION))
    {
        Serial.println("Long Press");
        buttonEventProcessed = true; // Mark as processed
        return LONG_PRESS;
    }

    // If button was released, it's a short press
    if (!buttonPressed)
    {
        // Ensure it wasn't a long press that was already handled
        if (now - buttonPressStartTime < LONG_PRESS_DURATION)
        {
            Serial.println("Short Press");
            buttonEventProcessed = true; // Mark as processed
            return SHORT_PRESS;
        }
        else
        {
            // This case handles when a long press is released.
            // The long press was already detected and returned, so we just clean up.
            buttonEventProcessed = true;
        }
    }

    return NO_PRESS;
}

void handleButtonEvents()
{
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
