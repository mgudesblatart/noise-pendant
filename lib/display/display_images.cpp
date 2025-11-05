// display_images.cpp
#include "display_images.h"
#include "../../include/thresholds.h"
#include <nvs_config.h>

float observed_max[3] = {11360000.0f, 11360000.0f, 11360000.0f};

void loadAllObservedMax() {
    for (int i = 0; i < 3; ++i) {
        observed_max[i] = loadObservedMax(i);
    }
}

float normalizeBarValue(float rms, int modeId) {
    // Update observed max for current mode and persist
    if (rms > observed_max[modeId]) {
        observed_max[modeId] = rms;
        saveObservedMax(modeId, rms);
    }
    float scaled = (rms / observed_max[modeId]) * 100.0f; // Map to 0-100
    float barValue = scaled / modeInfos[modeId].threshold;
    if (barValue > 1.0f) barValue = 1.0f;
    if (barValue < 0.0f) barValue = 0.0f;
    Serial.printf("Mode %d Observed Max: %.2f\n scaled: %.2f\n, threshold: %.2f\n, barValue: %.2f\n", modeId, observed_max[modeId], scaled, modeInfos[modeId].threshold, barValue);
    return barValue;
}

void resetObservedMax(int modeId) {
    observed_max[modeId] = 1.0f;
    saveObservedMax(modeId, 1.0f);
    for (int i = 0; i < 3; ++i) {
        observed_max[i] = loadObservedMax(i);
    }
}

void drawBarGraph(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2, float value) {
    int width = u8g2.getDisplayWidth();
    int height = u8g2.getDisplayHeight();
    int barWidth = (int)(value * width);
    u8g2.clearBuffer();
    u8g2.drawBox(0, 0, barWidth, height);
    u8g2.sendBuffer();
}

void drawFrame(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2) {
    int width = u8g2.getDisplayWidth();
    int height = u8g2.getDisplayHeight();
    u8g2.drawFrame(0, 0, width, height);
    u8g2.sendBuffer();
}

void drawCalibrationProgress(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2, float progress, int ellipsisFrame, int secondsElapsed) {
    int width = u8g2.getDisplayWidth();
    int height = u8g2.getDisplayHeight();
    int barWidth = (int)(progress * width); // Smooth progress bar
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(5, 15, "Calibrating");
    // Animated ellipses
    for (int i = 0; i < ellipsisFrame; ++i) {
        u8g2.drawStr(65 + 5*i, 15, ".");
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
void displayConfigUI(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2, int activeModeId) {
    int width = u8g2.getDisplayWidth();
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