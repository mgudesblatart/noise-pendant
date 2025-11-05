#pragma once
#include <U8g2lib.h>
#include "../../include/thresholds.h"

void drawBarGraph(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2, float value);
void drawFrame(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2);

// Normalization helper
float normalizeBarValue(float rms, int modeId);
void resetObservedMax(int modeId);
extern float observed_max[3];

// Calibration animation
void drawCalibrationProgress(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2, float progress, int ellipsisFrame, int secondsElapsed);

// Configuration UI display
void displayConfigUI(U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2, int activeModeId);
