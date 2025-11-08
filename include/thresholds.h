#pragma once

#include <Arduino.h>

#define DEBOUNCE_DURATION 50      // Debounce time in milliseconds
#define LONG_PRESS_DURATION 2000  // 2 seconds for a long press
#define ALARM_LEVEL 80.0          // Alarm threshold in dB
#define WARNING_LEVEL 70.0        // Warning threshold in dB
#define SAMPLE_BUFFER_SIZE 512
struct ModeInfo
{
    const char *name;
    const char *keyName;
    uint8_t glyph;
    float threshold;
    int id;
};

static const ModeInfo modeInfos[] = {
    {"MORNING", "MORN", 0x34, 25.0f, 0},    // Morning: sun behind cloud
    {"AFTERNOON", "AFTN", 0x33, 58.33f, 1}, // Afternoon: full sun
    {"NIGHT", "NGHT", 0x29, 37.5f, 2}       // Night: crescent moond
};

constexpr int NUM_MODES = sizeof(modeInfos) / sizeof(modeInfos[0]);

enum DisplayState
{
    DISPLAY_OFF,
    DISPLAY_BAR_GRAPH,
    DISPLAY_ALARM,
    DISPLAY_CONFIG,
    DISPLAY_SAVING,
    DISPLAY_CALIBRATION
};