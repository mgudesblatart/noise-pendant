#include "nvs_config.h"
#include "thresholds.h"

Preferences preferences;

int activeModeId = 0;
float currentThreshold = 0.0f;

void setupNVS() {
    preferences.begin(PREFS_NAMESPACE, false); // false = read/write mode
}

int loadCurrentMode() {
    // Use last mode in modeInfos as default
    activeModeId = preferences.getInt("Active_Mode_ID", modeInfos[0].id);
    currentThreshold = modeInfos[activeModeId].threshold;
    return activeModeId;
}

void saveCurrentMode() {
    preferences.putInt("Active_Mode_ID", activeModeId);
    for (int i = 0; i < sizeof(modeInfos)/sizeof(modeInfos[0]); ++i) {
        char key[32];
        snprintf(key, sizeof(key), "%s_Threshold_Rel", modeInfos[i].name);
        preferences.putFloat(key, modeInfos[i].threshold);
    }
}

// Observed max persistence for dynamic normalization
void saveObservedMax(int modeId, float value) {
    char key[32];
    snprintf(key, sizeof(key), "Observed_Max_%s", modeInfos[modeId].name);
    preferences.putFloat(key, value);
}

float loadObservedMax(int modeId) {
    char key[32];
    snprintf(key, sizeof(key), "Observed_Max_%s", modeInfos[modeId].name);
    return preferences.getFloat(key, 1.0f);
}

// Noise floor persistence
void saveNoiseFloor(int modeId, float value) {
    char key[32];
    snprintf(key, sizeof(key), "Noise_Floor_%s", modeInfos[modeId].name);
    preferences.putFloat(key, value);
}

float loadNoiseFloor(int modeId) {
    char key[32];
    snprintf(key, sizeof(key), "Noise_Floor_%s", modeInfos[modeId].name);
    return preferences.getFloat(key, 0.0f);
}