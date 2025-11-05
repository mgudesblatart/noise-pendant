#include <Arduino.h>
#include <U8g2lib.h>
#include "setup_display.h"
#include "update_display.h"
#include "display_images.h"
#include "setup_audio.h"
#include "audio_math.h"
#include "thresholds.h"
#include "nvs_config.h"
#include "button_interface.h"


// --- State Machine & Display Logic ---
#define BUILTIN_LED 8
#define SAMPLE_BUFFER_SIZE 512

U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 6, 5);
int32_t raw_samples[SAMPLE_BUFFER_SIZE];

DisplayState currentState = DISPLAY_OFF;
DisplayState lastState = DISPLAY_OFF;
bool alarmActive = false;
unsigned long alarmTriggeredAt = 0;

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
    return normalizeBarValue(averageRMS, activeModeId) * 100.0f;
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

// Dedicated alarm logic function
void updateAlarmLogic(float relativeNoiseLevel)
{
    lastState = currentState;
    if (alarmActive || relativeNoiseLevel >= 100.0f)
    {
        if (!alarmActive && relativeNoiseLevel >= 100.0f)
        {
            alarmActive = true;
            alarmTriggeredAt = millis();
            Serial.println("Alarm triggered!");
        }
        currentState = DISPLAY_ALARM;
    }
    else if (relativeNoiseLevel >= 25.0f)
    {
        currentState = DISPLAY_BAR_GRAPH;
        alarmActive = false;
    }
    else
    {
        currentState = DISPLAY_OFF;
        alarmActive = false;
    }
}

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
        updateAlarmLogic(relativeNoiseLevel);
        float adjustedRMS = averageRMS - noiseFloor;
        if (adjustedRMS < 0.0f)
            adjustedRMS = 0.0f;

        // Serial.printf("Mode: %s (%d), RMS: %.2f, Peak: %ld, DC Offset: %.2f, Avg RMS: %.2f, Observed Max: %.2f, Noise Floor: %.2f, Current Threshold: %.2f, relativeNoiseLevel: %.2f \n",
        //               modeInfos[activeModeId].name, activeModeId, last_rms, averageRMS, adjustedRMS, observed_max[activeModeId], noiseFloor, currentThreshold, relativeNoiseLevel);
        // Serial.printf("Alarm State: %s\n", alarmActive ? "ACTIVE" : "INACTIVE");

        vTaskDelay(10 / portTICK_PERIOD_MS); // yield to other tasks
    }
}

void displayTask(void *pvParameters)
{
    while (1)
    {
        float barValue = normalizeBarValue(averageRMS, activeModeId);
        updateDisplay(u8g2, currentState, barValue);
        vTaskDelay(100 / portTICK_PERIOD_MS); // update every 100ms
    }
}

void testConfigDisplay()
{
    for (int i = 0; i < 3; ++i)
    {
        displayConfigUI(u8g2, i);
        // Serial.printf("Displayed config UI for mode %d: %s, threshold %.1f\n", i, modeInfos[i].name, modeInfos[i].threshold);
        delay(1500); // Show each mode for 1.5 seconds
    }
}

void setup()
{
    delay(1000);
    Serial.begin(115200);
    pinMode(BUILTIN_LED, OUTPUT);
    setupDisplay(u8g2);
    setupAudio();
    setupNVS();
    int current_mode = loadCurrentMode();
    calibrateNoiseFloor(current_mode);
    setupButton();
    displayConfigUI(u8g2, current_mode); // Show current mode after test
    delay(1500);
    Serial.println("Init complete. Starting FreeRTOS tasks.");
    xTaskCreatePinnedToCore(audioTask, "AudioTask", 4096, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(displayTask, "DisplayTask", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(buttonTask, "ButtonTask", 2048, NULL, 1, NULL, 1);
}

void loop()
{
    vTaskDelay(1000 / portTICK_PERIOD_MS); // main loop does nothing
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
