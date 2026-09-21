#include "buzzer.h"
#include <Arduino.h>
#include "../config.h"

static uint8_t currentVolume = 64;

void buzzer_init() {
    ledcSetup(BUZZER_LEDC_CHANNEL, 440, BUZZER_LEDC_RES);
    ledcAttachPin(PIN_BUZZER, BUZZER_LEDC_CHANNEL);
    ledcWrite(BUZZER_LEDC_CHANNEL, 0);
}

void buzzer_setVolume(uint8_t volume) {
    if (volume > BUZZER_MAX_DUTY) volume = BUZZER_MAX_DUTY;
    currentVolume = volume;
}

void buzzer_tone(float freq, uint8_t volume) {
    if (freq < 20.0f || freq > 20000.0f) {
        buzzer_off();
        return;
    }
    if (volume > BUZZER_MAX_DUTY) volume = BUZZER_MAX_DUTY;
    currentVolume = volume;

    ledcWriteTone(BUZZER_LEDC_CHANNEL, freq);
    ledcWrite(BUZZER_LEDC_CHANNEL, volume);
}

void buzzer_off() {
    ledcWrite(BUZZER_LEDC_CHANNEL, 0);
}