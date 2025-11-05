// display_images.cpp
#include "display_images.h"
#include "thresholds.h"
#include "nvs_config.h"

void clearScreen(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2)
{
    u8g2.setDrawColor(1);
    u8g2.clearBuffer();
    u8g2.sendBuffer();
}

void drawAlarm(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2)
{
    // Flashing effect: alternate between triangle and blank every 500ms
    if (((millis() / 500) % 2) == 0)
    {
        u8g2.clearBuffer();
        int w = u8g2.getDisplayWidth();
        int h = u8g2.getDisplayHeight();
        u8g2.setDrawColor(1);
        u8g2.drawTriangle(w / 2, 5, w / 2 - 16, h - 5, w / 2 + 16, h - 5);
        u8g2.setFont(u8g2_font_fub14_tr); // Use a larger, bold font
        u8g2.setDrawColor(0);
        int exMarkWidth = u8g2.getStrWidth("!");
        int exMarkHeight = 14; // Approximate height for fub14
        int triCenterY = (5 + (h - 5) + (h - 5)) / 3;
        int exMarkX = w / 2 - exMarkWidth / 2;
        int exMarkY = triCenterY + exMarkHeight / 2;
        u8g2.drawStr(exMarkX, exMarkY, "!");
        u8g2.sendBuffer();
    }
    else
    {
        clearScreen(u8g2);
    }
}

void drawBarGraph(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2, float value)
{
    int width = u8g2.getDisplayWidth();
    int height = u8g2.getDisplayHeight();
    int barWidth = (int)(value * width);
    u8g2.setDrawColor(1);
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 10, "BAR");
    u8g2.drawBox(0, 0, barWidth, height);
    u8g2.sendBuffer();
}

void drawFrame(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2)
{
    int width = u8g2.getDisplayWidth();
    int height = u8g2.getDisplayHeight();
    u8g2.setDrawColor(1);
    u8g2.drawFrame(0, 0, width, height);
    u8g2.sendBuffer();
}

void drawCalibrationProgress(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2, float progress, int ellipsisFrame, int secondsElapsed)
{
    int width = u8g2.getDisplayWidth();
    int height = u8g2.getDisplayHeight();
    int barWidth = (int)(progress * width); // Smooth progress bar
    u8g2.setDrawColor(1);
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(5, 15, "Calibrating");
    // Animated ellipses
    for (int i = 0; i < ellipsisFrame; ++i)
    {
        u8g2.drawStr(65 + 5 * i, 15, ".");
    }
    // Progress bar (smooth)
    u8g2.drawFrame(0, 25, width, 10);
    u8g2.drawBox(0, 25, barWidth, 10);
    // Seconds indicator as text
    char secStr[8];
    snprintf(secStr, sizeof(secStr), "%ds", secondsElapsed);
    u8g2.drawStr(width - 20, 40, secStr);
    u8g2.sendBuffer();
}

// Configuration UI display
void displayConfigUI(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2, int activeModeId)
{
    int width = u8g2.getDisplayWidth();
    u8g2.setDrawColor(1);
    u8g2.clearBuffer();
    // Draw mode icon centered
    u8g2.setFont(u8g2_font_unifont_t_weather);
    int iconWidth = 15; // from font BBX Width
    int iconX = (width - iconWidth) / 2;
    u8g2.drawGlyph(iconX, 22, modeInfos[activeModeId].glyph);
    // Draw mode name centered below icon
    u8g2.setFont(u8g2_font_6x10_tf);
    int nameWidth = u8g2.getStrWidth(modeInfos[activeModeId].name);
    int nameX = (width - nameWidth) / 2;
    u8g2.drawStr(nameX, 39, modeInfos[activeModeId].name);
    u8g2.sendBuffer();
}