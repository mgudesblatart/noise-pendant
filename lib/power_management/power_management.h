#pragma once

#include <Arduino.h>
#include "esp_pm.h"
#include "esp_sleep.h"
#include <stdint.h>

// Power management setup
void setupPowerManagement();

// Light sleep functions
void enterLightSleep(uint32_t sleepTimeMs);
void configureSleepWakeup(uint32_t wakeupTimeMs);
void enableGPIOWakeup(gpio_num_t gpio_pin);

// Wakeup event handling
esp_sleep_wakeup_cause_t getWakeupCause();
void logWakeupReason();
