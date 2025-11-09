#pragma once

#include <Arduino.h>

// ============================================================================
// Hardware Configuration
// ============================================================================

// GPIO Pin Assignments
#define BUILTIN_LED GPIO_NUM_8
#define BUTTON_PIN GPIO_NUM_3

// I2S Audio Configuration
#define I2S_MIC_SERIAL_CLOCK GPIO_NUM_0
#define I2S_MIC_LEFT_RIGHT_CLOCK GPIO_NUM_2
#define I2S_MIC_SERIAL_DATA GPIO_NUM_1
#define SAMPLE_RATE 8000
#define SAMPLE_BUFFER_SIZE 512

// I2C Display Configuration
#define DISPLAY_SDA_PIN GPIO_NUM_5
#define DISPLAY_SCL_PIN GPIO_NUM_6

// ============================================================================
// Button Timing Constants
// ============================================================================

#define BUTTON_DEBOUNCE_MS 50      // Debounce time in milliseconds
#define BUTTON_LONG_PRESS_MS 2000  // Long press threshold in milliseconds

// ============================================================================
// Task Timing Constants
// ============================================================================

#define DISPLAY_UPDATE_MS 100      // Display refresh rate
#define BUTTON_POLL_MS 10          // Button polling interval
#define AUDIO_TASK_YIELD_MS 10     // Audio task yield to other tasks
#define MAIN_LOOP_DELAY_MS 1000    // Main loop delay
#define LIGHT_SLEEP_DURATION_MS 50 // Sleep duration in quiet monitoring state

// ============================================================================
// Audio Processing Constants
// ============================================================================

#define RMS_HISTORY_SIZE 16        // Number of RMS samples to average
#define CALIBRATION_DURATION_MS 5000  // Noise floor calibration time

// ============================================================================
// Audio Thresholds (deprecated - moved to ModeInfo)
// ============================================================================

#define ALARM_LEVEL 80.0           // Alarm threshold in dB (unused)
#define WARNING_LEVEL 70.0         // Warning threshold in dB (unused)

// ============================================================================
// Task Stack Sizes
// ============================================================================

#define AUDIO_TASK_STACK_SIZE 4096
#define DISPLAY_TASK_STACK_SIZE 4096
#define BUTTON_TASK_STACK_SIZE 2048

// ============================================================================
// State Machine Timing
// ============================================================================

#define CONFIG_MODE_TIMEOUT_MS 5000  // Config mode auto-exit timeout
