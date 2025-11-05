#include "setup_display.h"

void setupDisplay(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2) {
    u8g2.begin();
    u8g2.setContrast(255);
    u8g2.setBusClock(400000);
    u8g2.setFont(u8g2_font_ncenB10_tr);
}
