#pragma once
#include <U8g2lib.h>
#include "thresholds.h"

void clearScreen(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2);
void drawBarGraph(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2, float value);
void drawAlarm(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2);
void drawFrame(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2);

// Display power management
void displaySleep(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2);
void displayWake(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2);

// Calibration animation
void drawCalibrationProgress(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2, float progress, int ellipsisFrame, int secondsElapsed);

// Configuration UI display
void displayConfigUI(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2, int activeModeId, bool inverted = false);

// Saving config animation
void drawSavingConfigUI(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2, int activeModeId);
