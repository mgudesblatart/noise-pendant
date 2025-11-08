#include <Arduino.h>
#include <math.h>
#include <driver/i2s.h>
#include "thresholds.h"
#include "audio_math.h"
#include "audio_state.h"
#include "nvs_config.h"
#include "state_machine.h"
#include "display_images.h"

#define RMS_HISTORY_SIZE 16

float rms_history[RMS_HISTORY_SIZE];
int rms_history_index = 0;

void removeDCOffset(int32_t* samples, int count, AudioState& audioState) {
    int64_t sum = 0;
    for (int i = 0; i < count; i++) {
        sum += samples[i];
    }
    float dc = (float)sum / count;
    audioState.setLastDCOffset(dc);
    for (int i = 0; i < count; i++) {
        samples[i] -= (int32_t)dc;
    }
}

void processAudioBlock(int32_t* samples, int count, AudioState& audioState) {
    removeDCOffset(samples, count, audioState);
    int64_t sum_squares = 0;
    int32_t peak = 0;
    for (int i = 0; i < count; i++) {
        int32_t abs_sample = abs(samples[i]);
        if (abs_sample > peak) peak = abs_sample;
        sum_squares += (int64_t)samples[i] * samples[i];
    }
    float rms = sqrt((float)sum_squares / count);
    audioState.setLastRMS(rms);
    audioState.setLastPeak(peak);
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

// Moved functions from main.ino
float normalizeBarValue(float rms, int modeId, const AudioState& audioState) {
    float scaled = (rms / audioState.getObservedMax(modeId)) * 100.0f; // Map to 0-100
    float barValue = scaled / modeInfos[modeId].threshold;
    if (barValue > 1.0f)
        barValue = 1.0f;
    if (barValue < 0.0f)
        barValue = 0.0f;
    return barValue;
}

float getRelativeLoudness(const AudioState& audioState, StateMachine& stateMachine) {
    return normalizeBarValue(audioState.getAverageRMS(), stateMachine.getActiveModeId(), audioState) * 100.0f;
}

void resetObservedMax(int modeId, AudioState& audioState) {
    audioState.setObservedMax(modeId, 1.0f);
    saveObservedMax(modeId, 1.0f);
    audioState.loadFromNVS();
}

void calibrateNoiseFloor(int modeId, int32_t* raw_samples, size_t sample_buffer_size, AudioState& audioState, U8G2_SSD1306_72X40_ER_F_HW_I2C &u8g2) {
    Serial.println("Calibrating noise floor...");
    unsigned long start = millis();
    float maxRMS = 0.0f;
    int frame = 0;
    while (millis() - start < 5000) {
        size_t bytes_read = 0;
        esp_err_t err = i2s_read(I2S_NUM_0, raw_samples, sizeof(int32_t) * SAMPLE_BUFFER_SIZE, &bytes_read, portMAX_DELAY);
        if (err != ESP_OK)
            continue;
        int samples_read = bytes_read / sizeof(int32_t);
        processAudioBlock(raw_samples, samples_read, audioState);
        float rms = getAverageRMS();
        if (rms > audioState.getObservedMax(modeId)) {
            audioState.setObservedMax(modeId, rms);
            saveObservedMax(modeId, rms);
        }
        float progress = (float)(millis() - start) / 5000.0f;
        int secondsElapsed = (millis() - start) / 1000;
        drawCalibrationProgress(u8g2, progress, (frame % 4) + 1, secondsElapsed);
        frame++;
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    audioState.setNoiseFloor(maxRMS * 1.2f);
    saveNoiseFloor(modeId, audioState.getNoiseFloor());
}

void processAudioTask(int32_t* raw_samples, size_t sample_buffer_size, AudioState& audioState, StateMachine& stateMachine) {
    size_t bytes_read = 0;
    esp_err_t err = i2s_read(I2S_NUM_0, raw_samples, sizeof(int32_t) * sample_buffer_size, &bytes_read, portMAX_DELAY);
    if (err != ESP_OK) {
        Serial.printf("I2S read failed: %d\n", err);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        return;
    }
    int samples_read = bytes_read / sizeof(int32_t);
    processAudioBlock(raw_samples, samples_read, audioState);
    audioState.setAverageRMS(getAverageRMS());
    float relativeNoiseLevel = getRelativeLoudness(audioState, stateMachine);
    stateMachine.update(relativeNoiseLevel);
    float adjustedRMS = audioState.getAverageRMS() - audioState.getNoiseFloor();
    if (adjustedRMS < 0.0f)
        adjustedRMS = 0.0f;

    vTaskDelay(10 / portTICK_PERIOD_MS); // yield to other tasks
}
