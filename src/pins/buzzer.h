#pragma once
#include <stdint.h>

void buzzer_init();
void buzzer_tone(float freq, uint8_t volume);
void buzzer_off();
void buzzer_setVolume(uint8_t volume);