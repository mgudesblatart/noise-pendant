#include "update_display.h"
#include "display_images.h"
#include <Arduino.h>

void updateDisplay(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2, DisplayState state, float value) {
    switch (state) {
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
    }
}
