

#include <driver/gpio.h>
#include "power_management.h"

void setupPowerManagement()
{
    // Configure dynamic frequency scaling for the ESP32-C3
    esp_pm_config_esp32c3_t pm_config = {
        .max_freq_mhz = 160,       // Maximum CPU frequency
        .min_freq_mhz = 80,        // Minimum CPU frequency during idle
        .light_sleep_enable = true // Enable automatic light sleep
    };

    // Apply the power management configuration
    esp_pm_configure(&pm_config);
}

void configureSleepWakeup(uint32_t wakeupTimeMs)
{
    // Configure timer to wake from sleep after specified duration
    // Convert milliseconds to microseconds for the ESP32 sleep API
    esp_sleep_enable_timer_wakeup(wakeupTimeMs * 1000ULL);
}

void enterLightSleep(uint32_t sleepTimeMs)
{
    // Configure the wakeup timer
    configureSleepWakeup(sleepTimeMs);

    // Enter light sleep mode
    // The CPU will stop, but RAM and peripherals are retained
    // System will wake up after the timer expires or on other configured wakeup sources
    //
    // NOTE: This function should NOT be called when:
    // - I2S is actively reading (it will pause I2S transfers)
    // - Display needs to update frequently
    // - Button events need immediate response
    //
    // For FreeRTOS tasks, prefer using vTaskDelay() which allows automatic
    // light sleep via the power management configuration instead.

    Serial.println("Entering light sleep...");
    int64_t sleep_start = esp_timer_get_time();

    esp_light_sleep_start();

    // Calculate actual sleep duration
    int64_t sleep_end = esp_timer_get_time();
    int64_t actual_sleep_us = sleep_end - sleep_start;

    Serial.printf("Woke from light sleep after %lld us (target: %u ms)\n",
                  actual_sleep_us, sleepTimeMs);

    // Log the wakeup reason
    logWakeupReason();
}

void enableGPIOWakeup(gpio_num_t gpio_pin)
{
    // Enable wakeup from light sleep on GPIO pin level change
    // This allows button presses to wake the system from light sleep
    esp_sleep_enable_gpio_wakeup();
    gpio_wakeup_enable(gpio_pin, GPIO_INTR_LOW_LEVEL); // Wake on button press (active low)
}

esp_sleep_wakeup_cause_t getWakeupCause()
{
    return esp_sleep_get_wakeup_cause();
}

void logWakeupReason()
{
    esp_sleep_wakeup_cause_t wakeup_reason = getWakeupCause();

    switch (wakeup_reason)
    {
        case ESP_SLEEP_WAKEUP_TIMER:
            Serial.println("Wakeup caused by timer");
            break;
        case ESP_SLEEP_WAKEUP_GPIO:
            Serial.println("Wakeup caused by GPIO");
            break;
        case ESP_SLEEP_WAKEUP_UART:
            Serial.println("Wakeup caused by UART");
            break;
        case ESP_SLEEP_WAKEUP_UNDEFINED:
            Serial.println("Wakeup cause undefined (normal boot or reset)");
            break;
        default:
            Serial.printf("Wakeup caused by: %d\n", wakeup_reason);
            break;
    }
}
