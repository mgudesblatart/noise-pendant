#include <Arduino.h>
#include "nvs_config.h"
#include "update_display.h"
#include "display_images.h"
#include "state_machine.h"

void updateDisplay(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2, DisplayState state, float value, StateMachine& stateMachine)
{
    // Serial.printf("[DEBUG] updateDisplay: state=%d, value=%.2f\n", state, value);
    switch (state)
    {
    case DISPLAY_OFF:
        u8g2.clearBuffer();
        u8g2.sendBuffer();
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
}
