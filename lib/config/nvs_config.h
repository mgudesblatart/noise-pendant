#pragma once
#include <Preferences.h>
#include "thresholds.h"

#define PREFS_NAMESPACE "medallion"

struct CurrentModeData {
    int modeId;
    float currentThreshold;
};

void setupNVS();
void saveCurrentMode(int modeId);
CurrentModeData loadCurrentMode();

// Observed max persistence for dynamic normalization
void saveObservedMax(int modeId, float value);
float loadObservedMax(int modeId);

// Noise floor persistence
void saveNoiseFloor(int modeId, float value);
float loadNoiseFloor(int modeId);

extern Preferences preferences;