#pragma once
#include <driver/i2s.h>

void setupAudio();
extern i2s_config_t i2s_config;
extern i2s_pin_config_t i2s_mic_pins;
