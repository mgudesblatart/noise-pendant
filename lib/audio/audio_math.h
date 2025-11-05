#pragma once
#include <stdint.h>

void removeDCOffset(int32_t* samples, int count);
void processAudioBlock(int32_t* samples, int count);
float getAverageRMS();
extern float last_rms;
extern int32_t last_peak;
extern float last_dc_offset;
