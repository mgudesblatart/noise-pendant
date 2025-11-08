#include "nvs_config.h"
#include "thresholds.h"

Preferences preferences;

void setupNVS()
{
    preferences.begin(PREFS_NAMESPACE, false); // false = read/write mode
}

CurrentModeData loadCurrentMode()
{
    int modeId = preferences.getInt("Active_Mode_ID", modeInfos[0].id);
    float currentThreshold = modeInfos[modeId].threshold;
    return {modeId, currentThreshold};
}

void saveCurrentMode(int modeId)
{
    Serial.printf("Saving current mode: %s (%d)\n", modeInfos[modeId].name, modeId);
    preferences.putInt("Active_Mode_ID", modeId);
}

// Observed max persistence for dynamic normalization
void saveObservedMax(int modeId, float value)
{
    char key[32];
    snprintf(key, sizeof(key), "OM_%s", modeInfos[modeId].keyName);
    preferences.putFloat(key, value);
}

float loadObservedMax(int modeId)
{
    char key[32];
    snprintf(key, sizeof(key), "OM_%s", modeInfos[modeId].keyName);
    return preferences.getFloat(key, 1.0f);
}

// Noise floor persistence
void saveNoiseFloor(int modeId, float value)
{
    char key[32];
    snprintf(key, sizeof(key), "NF_%s", modeInfos[modeId].keyName);
    preferences.putFloat(key, value);
}

float loadNoiseFloor(int modeId)
{
    char key[32];
    snprintf(key, sizeof(key), "NF_%s", modeInfos[modeId].keyName);
    return preferences.getFloat(key, 0.0f);
}