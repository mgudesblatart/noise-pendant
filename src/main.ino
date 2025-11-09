#include <Arduino.h>
#include <U8g2lib.h>
#include <esp_pm.h>
#include "constants.h"
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
#include "task_functions.h"

U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, DISPLAY_SCL_PIN, DISPLAY_SDA_PIN);
int32_t raw_samples[SAMPLE_BUFFER_SIZE];

// State machine
StateMachine stateMachine;

// Audio state
AudioState audioState;

// Shared context for tasks
TaskContext taskContext;

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
    setupPowerManagement();
    Serial.println("Init complete. Starting FreeRTOS tasks.");

    // Initialize task context
    taskContext.audioState = &audioState;
    taskContext.stateMachine = &stateMachine;
    taskContext.display = &u8g2;
    taskContext.lastDisplayState = DISPLAY_OFF;  // Initialize display state tracking

    xTaskCreatePinnedToCore(audioTask, "AudioTask", AUDIO_TASK_STACK_SIZE, &taskContext, 3, NULL, 0);
    xTaskCreatePinnedToCore(displayTask, "DisplayTask", DISPLAY_TASK_STACK_SIZE, &taskContext, 1, NULL, 1);
    xTaskCreatePinnedToCore(buttonTask, "ButtonTask", BUTTON_TASK_STACK_SIZE, &taskContext, 1, NULL, 1);
}

void loop()
{
    // Main loop: nothing needed, all logic is in tasks/state machine
    vTaskDelay(MAIN_LOOP_DELAY_MS / portTICK_PERIOD_MS);
}
