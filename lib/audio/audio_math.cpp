#include "audio_math.h"
#include <math.h>

#define RMS_HISTORY_SIZE 16

float rms_history[RMS_HISTORY_SIZE];
int rms_history_index = 0;
float last_rms = 0.0f;
int32_t last_peak = 0;
float last_dc_offset = 0.0f;

void removeDCOffset(int32_t* samples, int count) {
    int64_t sum = 0;
    for (int i = 0; i < count; i++) {
        sum += samples[i];
    }
    float dc = (float)sum / count;
    last_dc_offset = dc;
    for (int i = 0; i < count; i++) {
        samples[i] -= (int32_t)dc;
    }
}

void processAudioBlock(int32_t* samples, int count) {
    removeDCOffset(samples, count);
    int64_t sum_squares = 0;
    int32_t peak = 0;
    for (int i = 0; i < count; i++) {
        int32_t abs_sample = abs(samples[i]);
        if (abs_sample > peak) peak = abs_sample;
        sum_squares += (int64_t)samples[i] * samples[i];
    }
    float rms = sqrt((float)sum_squares / count);
    last_rms = rms;
    last_peak = peak;
    rms_history[rms_history_index] = rms;
    rms_history_index = (rms_history_index + 1) % RMS_HISTORY_SIZE;
}

float getAverageRMS() {
    float sum = 0.0f;
    for (int i = 0; i < RMS_HISTORY_SIZE; i++) {
        sum += rms_history[i];
    }
    return sum / RMS_HISTORY_SIZE;
}
