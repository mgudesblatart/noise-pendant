#pragma once

#include "thresholds.h"

class AudioState {
private:
    float averageRMS;
    float noiseFloor;
    float observedMax[NUM_MODES];

    // Debug/diagnostic values
    float lastRMS;
    int32_t lastPeak;
    float lastDCOffset;

public:
    AudioState();

    // Getters
    float getAverageRMS() const { return averageRMS; }
    float getNoiseFloor() const { return noiseFloor; }
    float getObservedMax(int modeId) const;
    float getLastRMS() const { return lastRMS; }
    int32_t getLastPeak() const { return lastPeak; }
    float getLastDCOffset() const { return lastDCOffset; }

    // Setters
    void setAverageRMS(float value) { averageRMS = value; }
    void setNoiseFloor(float value) { noiseFloor = value; }
    void setObservedMax(int modeId, float value);
    void setLastRMS(float value) { lastRMS = value; }
    void setLastPeak(int32_t value) { lastPeak = value; }
    void setLastDCOffset(float value) { lastDCOffset = value; }

    // Utility methods
    void loadFromNVS();
};
