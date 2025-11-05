#pragma once
#include <Preferences.h>
#include "../../include/thresholds.h"

#define PREFS_NAMESPACE "medallion"

void setupNVS();
void saveCurrentMode();
int loadCurrentMode();

// Observed max persistence for dynamic normalization
void saveObservedMax(int modeId, float value);
float loadObservedMax(int modeId);

// Noise floor persistence
void saveNoiseFloor(int modeId, float value);
float loadNoiseFloor(int modeId);

extern Preferences preferences;
