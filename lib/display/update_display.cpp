#include "update_display.h"
#include "display_images.h"

void updateDisplay(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2, float value) {
    drawBarGraph(u8g2, value);
}
