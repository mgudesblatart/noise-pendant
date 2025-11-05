#pragma once

struct ModeInfo {
    const char* name;
    uint8_t glyph;
    float threshold;
    int id;
};

static const ModeInfo modeInfos[] = {
    {"MORNING", 0x34, 25.0f, 0},      // Morning: sun behind cloud
    {"AFTERNOON", 0x33, 58.33f, 1},     // Afternoon: full sun
    {"NIGHT", 0x29, 37.5f, 2}    // Night: crescent moond
};

extern float observed_max[3];

enum DisplayState {
    DISPLAY_OFF,
    DISPLAY_BAR_GRAPH,
    DISPLAY_ALARM
};