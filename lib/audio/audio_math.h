#pragma once
#include <stdint.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include "state_machine.h"
#include "thresholds.h"
#include "audio_state.h"

// Declare functions moved from main.ino
float normalizeBarValue(float rms, int modeId, const AudioState& audioState);
float getRelativeLoudness(const AudioState& audioState, StateMachine& stateMachine);
void resetObservedMax(int modeId, AudioState& audioState);
void calibrateNoiseFloor(int modeId, int32_t* raw_samples, size_t sample_buffer_size, AudioState& audioState, U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2);
void processAudioTask(int32_t* raw_samples, size_t sample_buffer_size, AudioState& audioState, StateMachine& stateMachine);

void removeDCOffset(int32_t* samples, int count, AudioState& audioState);
void processAudioBlock(int32_t* samples, int count, AudioState& audioState);
float getAverageRMS();
