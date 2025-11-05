#pragma once
#include <U8g2lib.h>

enum DisplayState {
    DISPLAY_OFF,
    DISPLAY_BAR_GRAPH,
    DISPLAY_ALARM
};
// Unified display update: handles all states
void updateDisplay(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2, DisplayState state, float value = 0.0f);

