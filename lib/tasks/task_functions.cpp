#include "task_functions.h"
#include "constants.h"
#include "audio_math.h"
#include "update_display.h"
#include "button_interface.h"
#include "thresholds.h"
#include <Arduino.h>

extern int32_t raw_samples[SAMPLE_BUFFER_SIZE];

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
        float barValue = normalizeBarValue(ctx->audioState->getAverageRMS(),
                                           ctx->stateMachine->getActiveModeId(),
                                           *ctx->audioState);
        updateDisplay(*ctx->display, ctx->stateMachine->getCurrentState(),
                     barValue, *ctx->stateMachine);
        vTaskDelay(DISPLAY_UPDATE_MS / portTICK_PERIOD_MS);
    }
}

void buttonTask(void *pvParameters)
{
    TaskContext* ctx = (TaskContext*)pvParameters;
    while (1)
    {
        handleButtonEvents(*ctx->stateMachine);
        vTaskDelay(BUTTON_POLL_MS / portTICK_PERIOD_MS);
    }
}
