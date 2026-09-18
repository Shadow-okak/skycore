#include "buzzer.h"
#include <Arduino.h>
#include "../config.h"

void buzzer_init() {
    pinMode(PIN_BUZZER, OUTPUT);
    noTone(PIN_BUZZER);
}

void buzzer_tone(float freq) {
    if (freq < 20.0f || freq > 20000.0f) {
        noTone(PIN_BUZZER);
        return;
    }
    tone(PIN_BUZZER, (unsigned int)freq);
}

void buzzer_off() {
    noTone(PIN_BUZZER);
}