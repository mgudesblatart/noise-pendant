#include <U8g2lib.h>
#include "button_interface.h"
#include "thresholds.h"

volatile bool buttonPressed = false;
volatile unsigned long buttonPressStartTime = 0;
volatile bool buttonEventProcessed = true;

void IRAM_ATTR buttonISR() {
    bool currentState = digitalRead(BUTTON_PIN);
    unsigned long now = millis();
    if (currentState == LOW) {  // Button pressed (active low)
        buttonPressStartTime = now;
        buttonPressed = true;
        buttonEventProcessed = false;
    } else if (buttonPressed) {  // Button released
        buttonPressed = false;
    }
}

void setupButton() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, CHANGE);
}

ButtonPressType checkButtonPress() {
    if (buttonEventProcessed) {
        return NO_PRESS;
    }
    unsigned long now = millis();
    // If button is still pressed, check for long press
    if (buttonPressed && (now - buttonPressStartTime >= LONG_PRESS_DURATION)) {
        buttonEventProcessed = true;
        return LONG_PRESS;
    }
    // If button was released and not processed yet
    if (!buttonPressed && !buttonEventProcessed) {
        buttonEventProcessed = true;
        if (now - buttonPressStartTime < LONG_PRESS_DURATION) {
            return SHORT_PRESS;
        }
    }
    return NO_PRESS;
}

// Forward declarations for state variables (to be defined in main.ino)
extern bool alarmActive;
extern DisplayState currentState;
extern float getRelativeLoudness();
extern void clearScreen(U8G2_SSD1306_72X40_ER_F_HW_I2C&);
extern void updateDisplay(U8G2_SSD1306_72X40_ER_F_HW_I2C&, DisplayState, float);
extern U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2;

void handleButtonEvents() {
    ButtonPressType pressType = checkButtonPress();
    if (pressType == NO_PRESS) return;
    if (alarmActive && pressType == SHORT_PRESS) {
        // Acknowledge alarm
        alarmActive = false;
        currentState = DISPLAY_BAR_GRAPH;
        Serial.println("Alarm acknowledged by short press.");
    } else if (pressType == SHORT_PRESS) {
        Serial.println("Short press detected (no alarm active).");
        // Placeholder: config mode cycling, etc.
    } else if (pressType == LONG_PRESS) {
        Serial.println("Long press detected.");
        // Placeholder: config save, etc.
    }
}
