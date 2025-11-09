#include <Arduino.h>
#include <driver/i2s.h>
#include "constants.h"
#include "audio_math.h"
#include "update_display.h"
#include "button_interface.h"
#include "thresholds.h"
#include "power_management.h"
#include "task_functions.h"

extern int32_t raw_samples[SAMPLE_BUFFER_SIZE];

void audioTask(void *pvParameters)
{
    TaskContext *ctx = (TaskContext *)pvParameters;
    while (1)
    {
        // Process audio samples
        size_t bytes_read = 0;
        int32_t *samples = raw_samples;
        esp_err_t err = i2s_read(I2S_NUM_0, samples, sizeof(int32_t) * SAMPLE_BUFFER_SIZE, &bytes_read, portMAX_DELAY);

        if (err != ESP_OK)
        {
            Serial.printf("I2S read failed: %d\n", err);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        int samples_read = bytes_read / sizeof(int32_t);
        processAudioBlock(samples, samples_read, *ctx->audioState);
        ctx->audioState->setAverageRMS(getAverageRMS());
        float relativeNoiseLevel = getRelativeLoudness(*ctx->audioState, *ctx->stateMachine);
        ctx->stateMachine->update(relativeNoiseLevel);

        // Power management: Use longer delay in passive monitoring, shorter in active states
        DisplayState currentState = ctx->stateMachine->getCurrentState();
        if (currentState == DISPLAY_OFF)
        {
            // Passive monitoring: longer delay allows automatic light sleep
            vTaskDelay(LIGHT_SLEEP_DURATION_MS / portTICK_PERIOD_MS);
        }
        else
        {
            // Active states: shorter delay for responsiveness
            vTaskDelay(AUDIO_TASK_YIELD_MS / portTICK_PERIOD_MS);
        }
    }
}
void displayTask(void *pvParameters)
{
    TaskContext *ctx = (TaskContext *)pvParameters;
    while (1)
    {
        float barValue = normalizeBarValue(ctx->audioState->getAverageRMS(),
                                           ctx->stateMachine->getActiveModeId(),
                                           *ctx->audioState);
        updateDisplay(*ctx->display, ctx->stateMachine->getCurrentState(),
                      barValue, *ctx->stateMachine, ctx->lastDisplayState);
        vTaskDelay(DISPLAY_UPDATE_MS / portTICK_PERIOD_MS);
    }
}

void buttonTask(void *pvParameters)
{
    TaskContext *ctx = (TaskContext *)pvParameters;
    Serial.println("Button task started");
    while (1)
    {
        handleButtonEvents(*ctx->stateMachine);
        vTaskDelay(BUTTON_POLL_MS / portTICK_PERIOD_MS);
    }
}
