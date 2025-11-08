#include <Arduino.h>
#include <U8g2lib.h>
#include <esp_pm.h>
#include "state_machine.h"
#include "setup_display.h"
#include "update_display.h"
#include "display_images.h"
#include "setup_audio.h"
#include "audio_math.h"
#include "audio_state.h"
#include "thresholds.h"
#include "nvs_config.h"
#include "button_interface.h"
#include "power_management.h"

#define BUILTIN_LED 8

U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 6, 5);
int32_t raw_samples[SAMPLE_BUFFER_SIZE];

// State machine
StateMachine stateMachine;

// Audio state
AudioState audioState;

// Shared context for tasks
struct TaskContext {
    AudioState* audioState;
    StateMachine* stateMachine;
    U8G2_SSD1306_72X40_ER_F_HW_I2C* display;
};

TaskContext taskContext;

void audioTask(void *pvParameters)
{
    TaskContext* ctx = (TaskContext*)pvParameters;
    while (1)
    {
        processAudioTask(raw_samples, SAMPLE_BUFFER_SIZE, *ctx->audioState, *ctx->stateMachine);
    }
}

void displayTask(void *pvParameters)
{
    TaskContext* ctx = (TaskContext*)pvParameters;
    while (1)
    {
        float barValue = normalizeBarValue(ctx->audioState->getAverageRMS(), ctx->stateMachine->getActiveModeId(), *ctx->audioState);
        updateDisplay(*ctx->display, ctx->stateMachine->getCurrentState(), barValue, *ctx->stateMachine);
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
    Serial.println("Load Audio State from NVS...");
    audioState.loadFromNVS();
    Serial.println("Calibrate Noise Floor...");
    calibrateNoiseFloor(stateMachine.getActiveModeId(), raw_samples, SAMPLE_BUFFER_SIZE, audioState, u8g2);
    displayConfigUI(u8g2, stateMachine.getActiveModeId());
    delay(1500);
    Serial.println("Init complete. Starting FreeRTOS tasks.");

    // Initialize task context
    taskContext.audioState = &audioState;
    taskContext.stateMachine = &stateMachine;
    taskContext.display = &u8g2;

    xTaskCreatePinnedToCore(audioTask, "AudioTask", 4096, &taskContext, 3, NULL, 0);
    xTaskCreatePinnedToCore(displayTask, "DisplayTask", 4096, &taskContext, 1, NULL, 1);
    xTaskCreatePinnedToCore(buttonTask, "ButtonTask", 2048, &taskContext, 1, NULL, 1);
    setupPowerManagement();
}

void loop()
{
    // Main loop: nothing needed, all logic is in tasks/state machine
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

// Button event handler task
void buttonTask(void *pvParameters)
{
    TaskContext* ctx = (TaskContext*)pvParameters;
    while (1)
    {
        handleButtonEvents(*ctx->stateMachine);
        vTaskDelay(10 / portTICK_PERIOD_MS); // check button every 10ms
    }
}
