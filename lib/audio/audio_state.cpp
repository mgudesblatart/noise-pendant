#include "audio_state.h"
#include "nvs_config.h"
#include <Arduino.h>

AudioState::AudioState()
    : averageRMS(0.0f), noiseFloor(0.0f),
      lastRMS(0.0f), lastPeak(0), lastDCOffset(0.0f) {
    // Initialize observed max values
    for (int i = 0; i < NUM_MODES; i++) {
        observedMax[i] = 1.0f;
    }
}

float AudioState::getObservedMax(int modeId) const {
    if (modeId < 0 || modeId >= NUM_MODES) {
        return 1.0f; // Safe default
    }
    return observedMax[modeId];
}

void AudioState::setObservedMax(int modeId, float value) {
    if (modeId >= 0 && modeId < NUM_MODES) {
        observedMax[modeId] = value;
    }
}

void AudioState::loadFromNVS() {
    for (int i = 0; i < NUM_MODES; i++) {
        observedMax[i] = loadObservedMax(i);
    }
    // Note: noiseFloor is loaded per-mode during calibration
}
