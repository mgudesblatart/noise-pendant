#include <Arduino.h>
#include <U8g2lib.h>
#include "state_machine.h"
#include "setup_display.h"
#include "update_display.h"
#include "display_images.h"
#include "setup_audio.h"
#include "audio_math.h"
#include "thresholds.h"
#include "nvs_config.h"
#include "button_interface.h"


#define BUILTIN_LED 8
#define SAMPLE_BUFFER_SIZE 512

U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 6, 5);
int32_t raw_samples[SAMPLE_BUFFER_SIZE];

// State machine
StateMachine stateMachine;

float averageRMS = 0.0f;
float noiseFloor = 0.0f;
float observed_max[3] = {11360000.0f, 11360000.0f, 11360000.0f};



float normalizeBarValue(float rms, int modeId)
{
    float scaled = (rms / observed_max[modeId]) * 100.0f; // Map to 0-100
    float barValue = scaled / modeInfos[modeId].threshold;
    if (barValue > 1.0f)
        barValue = 1.0f;
    if (barValue < 0.0f)
        barValue = 0.0f;
    // Serial.printf("Mode %d, Observed Max: %.2f scaled: %.2f threshold: %.2f barValue: %.2f\n", modeId, observed_max[modeId], scaled, modeInfos[modeId].threshold, barValue);
    return barValue;
}

// Provide a stub for getRelativeLoudness (replace with real function if available)
float getRelativeLoudness() {
    // For now, just return the current bar value
    return normalizeBarValue(averageRMS, stateMachine.getActiveModeId()) * 100.0f;
}

void resetObservedMax(int modeId)
{
    observed_max[modeId] = 1.0f;
    saveObservedMax(modeId, 1.0f);
    for (int i = 0; i < 3; ++i)
    {
        observed_max[i] = loadObservedMax(i);
    }
}
// Calibrate noise floor for current mode
void calibrateNoiseFloor(int modeId)
{
    Serial.println("Calibrating noise floor...");
    unsigned long start = millis();
    float maxRMS = 0.0f;
    int frame = 0;
    while (millis() - start < 5000)
    {
        size_t bytes_read = 0;
        esp_err_t err = i2s_read(I2S_NUM_0, raw_samples, sizeof(int32_t) * SAMPLE_BUFFER_SIZE, &bytes_read, portMAX_DELAY);
        if (err != ESP_OK)
            continue;
        int samples_read = bytes_read / sizeof(int32_t);
        processAudioBlock(raw_samples, samples_read);
        float rms = getAverageRMS();
        // Update observed max for current mode and persist
        if (rms > observed_max[modeId])
        {
            observed_max[modeId] = rms;
            saveObservedMax(modeId, rms);
        }
        float progress = (float)(millis() - start) / 5000.0f;
        int secondsElapsed = (millis() - start) / 1000;
        drawCalibrationProgress(u8g2, progress, (frame % 4) + 1, secondsElapsed);
        frame++;
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    // Add a small buffer to the noise floor
    noiseFloor = maxRMS * 1.2f;
    saveNoiseFloor(modeId, noiseFloor);
    // Serial.printf("Noise floor for mode %d calibrated: %.2f\n", modeId, noiseFloor);
}

// Alarm logic is now handled by StateMachine

void audioTask(void *pvParameters)
{
    while (1)
    {
        size_t bytes_read = 0;
        esp_err_t err = i2s_read(I2S_NUM_0, raw_samples, sizeof(int32_t) * SAMPLE_BUFFER_SIZE, &bytes_read, portMAX_DELAY);
        if (err != ESP_OK)
        {
            Serial.printf("I2S read failed: %d\n", err);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        int samples_read = bytes_read / sizeof(int32_t);
        processAudioBlock(raw_samples, samples_read);
        averageRMS = getAverageRMS();
        float relativeNoiseLevel = getRelativeLoudness();
        stateMachine.update(relativeNoiseLevel);
        float adjustedRMS = averageRMS - noiseFloor;
        if (adjustedRMS < 0.0f)
            adjustedRMS = 0.0f;

        vTaskDelay(10 / portTICK_PERIOD_MS); // yield to other tasks
    }
}

void displayTask(void *pvParameters)
{
    while (1)
    {
        float barValue = normalizeBarValue(averageRMS, stateMachine.getActiveModeId());
        updateDisplay(u8g2, stateMachine.getCurrentState(), barValue);
        vTaskDelay(100 / portTICK_PERIOD_MS); // update every 100ms
    }
}

void testConfigDisplay()
{
    for (int i = 0; i < 3; ++i)
    {
        displayConfigUI(u8g2, i);
        delay(1500); // Show each mode for 1.5 seconds
    }
}

// Config mode logic is now in StateMachine

void setup()
{
    delay(1000);
    Serial.begin(115200);
    pinMode(BUILTIN_LED, OUTPUT);
    Serial.println("Setup Display...");
    setupDisplay(u8g2);
    Serial.println("Setup Audio...");
    setupAudio();
    Serial.println("Setup NVS...");
    setupNVS();
    Serial.println("Setup Buttons...");
    setupButton();
    Serial.println("Begin State Machine...");
    stateMachine.begin();
    Serial.println("Calibrate Noise Floor...");
    calibrateNoiseFloor(stateMachine.getActiveModeId());
    displayConfigUI(u8g2, stateMachine.getActiveModeId());
    delay(1500);
    Serial.println("Init complete. Starting FreeRTOS tasks.");
    xTaskCreatePinnedToCore(audioTask, "AudioTask", 4096, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(displayTask, "DisplayTask", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(buttonTask, "ButtonTask", 2048, NULL, 1, NULL, 1);
}

void loop()
{
    // Main loop: nothing needed, all logic is in tasks/state machine
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

// Button event handler task
void buttonTask(void *pvParameters)
{
    while (1)
    {
        handleButtonEvents();
        vTaskDelay(10 / portTICK_PERIOD_MS); // check button every 10ms
    }
}
