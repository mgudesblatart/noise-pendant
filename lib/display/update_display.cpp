#include <Arduino.h>
#include "nvs_config.h"
#include "update_display.h"
#include "display_images.h"
#include "state_machine.h"

void updateDisplay(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2, DisplayState state, float value, StateMachine& stateMachine, DisplayState& lastState)
{
    // Serial.printf("[DEBUG] updateDisplay: state=%d, value=%.2f\n", state, value);

    // Wake display if transitioning from DISPLAY_OFF to any other state
    if (lastState == DISPLAY_OFF && state != DISPLAY_OFF)
    {
        displayWake(u8g2);
    }

    switch (state)
    {
    case DISPLAY_OFF:
        // Only put display to sleep if we weren't already in DISPLAY_OFF
        if (lastState != DISPLAY_OFF)
        {
            displaySleep(u8g2);
        }
        break;
    case DISPLAY_BAR_GRAPH:
        drawBarGraph(u8g2, value);
        break;
    case DISPLAY_ALARM:
        drawAlarm(u8g2);
        break;
    case DISPLAY_CONFIG:
        displayConfigUI(u8g2, stateMachine.getActiveModeId());
        break;
    case DISPLAY_SAVING:
        drawSavingConfigUI(u8g2, stateMachine.getActiveModeId());
        break;
    }

    lastState = state;
}
