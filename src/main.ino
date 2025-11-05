#include <Arduino.h>
#include <U8g2lib.h>
#include <setup_display.h>
#include <update_display.h>
#include <display_images.h>
#include <setup_audio.h>
#include <audio_math.h>
#include "../include/thresholds.h"
#include <nvs_config.h>

#define BUILTIN_LED 8
#define SAMPLE_BUFFER_SIZE 512

U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 6, 5);
int32_t raw_samples[SAMPLE_BUFFER_SIZE];

// Active mode and threshold (should be loaded from NVS in future)
int activeModeId = 0;
float currentThreshold = modeInfos[0].threshold;
float noiseFloor[3] = {0.0f, 0.0f, 0.0f};
// Calibrate noise floor for current mode
void calibrateNoiseFloor(int modeId) {
    Serial.println("Calibrating noise floor...");
    unsigned long start = millis();
    float maxRMS = 0.0f;
    int frame = 0;
    while (millis() - start < 5000) {
        size_t bytes_read = 0;
        esp_err_t err = i2s_read(I2S_NUM_0, raw_samples, sizeof(int32_t) * SAMPLE_BUFFER_SIZE, &bytes_read, portMAX_DELAY);
        if (err != ESP_OK) continue;
        int samples_read = bytes_read / sizeof(int32_t);
        processAudioBlock(raw_samples, samples_read);
        float rms = getAverageRMS();
        if (rms > maxRMS) maxRMS = rms;
        float progress = (float)(millis() - start) / 5000.0f;
        int secondsElapsed = (millis() - start) / 1000;
        drawCalibrationProgress(u8g2, progress, (frame % 4) + 1, secondsElapsed);
        frame++;
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    // Add a small buffer to the noise floor
    maxRMS *= 1.2f;
    noiseFloor[modeId] = maxRMS;
    saveNoiseFloor(modeId, maxRMS);
    Serial.printf("Noise floor for mode %d calibrated: %.2f\n", modeId, maxRMS);
}


void audioTask(void *pvParameters) {
    while (1) {
        size_t bytes_read = 0;
        esp_err_t err = i2s_read(I2S_NUM_0, raw_samples, sizeof(int32_t) * SAMPLE_BUFFER_SIZE, &bytes_read, portMAX_DELAY);
        if (err != ESP_OK) {
            Serial.printf("I2S read failed: %d\n", err);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        int samples_read = bytes_read / sizeof(int32_t);
        processAudioBlock(raw_samples, samples_read);
        // Offset RMS by noise floor
        float adjustedRMS = getAverageRMS() - noiseFloor[activeModeId];
        if (adjustedRMS < 0.0f) adjustedRMS = 0.0f;
        Serial.printf("Mode: %s (%d), RMS: %.2f, Peak: %ld, DC Offset: %.2f, Avg RMS: %.2f, Observed Max: %.2f, Noise Floor: %.2f\n",
            modeInfos[activeModeId].name, activeModeId, last_rms, last_peak, last_dc_offset, adjustedRMS, observed_max[activeModeId], noiseFloor[activeModeId]);
        Serial.printf("Alarm State: %s\n", "(not implemented)");
        vTaskDelay(10 / portTICK_PERIOD_MS); // yield to other tasks
    }
}

void displayTask(void *pvParameters) {
    while (1) {
        float barValue = normalizeBarValue(getAverageRMS(), activeModeId);
        Serial.printf("Current Threshold: %.2f, Bar Value: %.2f\n", modeInfos[activeModeId].threshold, barValue);
        updateDisplay(u8g2, barValue);
        vTaskDelay(100 / portTICK_PERIOD_MS); // update every 100ms
    }
}

void testConfigDisplay() {
    for (int i = 0; i < 3; ++i) {
        displayConfigUI(u8g2, i);
        Serial.printf("Displayed config UI for mode %d: %s, threshold %.1f\n", i, modeInfos[i].name, modeInfos[i].threshold);
        delay(1500); // Show each mode for 1.5 seconds
    }
}

void setup() {
    delay(1000);
    Serial.begin(115200);
    pinMode(BUILTIN_LED, OUTPUT);
    setupDisplay(u8g2);
    setupAudio();
    setupNVS();
    int current_mode = loadCurrentMode();
    // // Load noise floor from NVS, calibrate if not set
    // for (int i = 0; i < 3; ++i) {
    //     noiseFloor[i] = loadNoiseFloor(i);
    //     if (noiseFloor[i] < 1.0f) {
    //         calibrateNoiseFloor(i);
    //     }
    // }
    calibrateNoiseFloor(current_mode);
    Serial.println("Init complete. Starting FreeRTOS tasks.");
    testConfigDisplay(); // <--- Test the config UI display
    xTaskCreatePinnedToCore(audioTask, "AudioTask", 4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(displayTask, "DisplayTask", 4096, NULL, 1, NULL, 1);
}

void loop() {
    vTaskDelay(1000 / portTICK_PERIOD_MS); // main loop does nothing
}
