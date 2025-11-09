#pragma once

#include "audio_state.h"
#include "state_machine.h"
#include <U8g2lib.h>

// Shared context for FreeRTOS tasks
struct TaskContext {
    AudioState* audioState;
    StateMachine* stateMachine;
    U8G2_SSD1306_72X40_ER_F_HW_I2C* display;
    DisplayState lastDisplayState;  // Track display state transitions for power management
};

// Task function declarations
void audioTask(void *pvParameters);
void displayTask(void *pvParameters);
void buttonTask(void *pvParameters);
