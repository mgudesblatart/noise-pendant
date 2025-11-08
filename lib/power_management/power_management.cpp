#include "power_management.h"

void setupPowerManagement() {
    // Configure dynamic frequency scaling for the ESP32-C3
    esp_pm_config_esp32c3_t pm_config = {
        .max_freq_mhz = 160,  // Maximum CPU frequency
        .min_freq_mhz = 80,   // Minimum CPU frequency during idle
        .light_sleep_enable = true  // Enable automatic light sleep
    };

    // Apply the power management configuration
    esp_pm_configure(&pm_config);
}