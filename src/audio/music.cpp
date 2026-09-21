#include "music.h"
#include <Arduino.h>
#include <math.h>
#include "buzzer.h"
#include "config.h"
#include "settings.h"
#include "core/physics.h"

struct Note {
    float freq;
    uint32_t durMs;
};

const Note PLANET_MELODY[] = {
    { 220.00f, 600 },
    {    0.0f, 100 },
    { 261.63f, 500 },
    { 293.66f, 500 },
    {    0.0f, 200 },
    { 246.94f, 500 },
    { 220.00f, 700 },
    { 196.00f, 500 },
    {    0.0f, 800 },
};
constexpr int PLANET_LEN = sizeof(PLANET_MELODY) / sizeof(Note);

const Note ORBIT_MELODY[] = {
    { 261.63f, 500 },
    {    0.0f, 800 },
    { 329.63f, 500 },
    {    0.0f, 900 },
    { 293.66f, 500 },
    {    0.0f, 900 },
    { 392.00f, 700 },
    {    0.0f, 1400 },
};
constexpr int ORBIT_LEN = sizeof(ORBIT_MELODY) / sizeof(Note);

const Note KARMAN_MELODY[] = {
    {  261.63f, 300 },
    {  329.63f, 300 },
    {  392.00f, 300 },
    {  523.25f, 300 },
    {  659.25f, 300 },
    {  783.99f, 500 },
    { 1046.50f, 500 },
    { 1318.51f, 500 },
    { 1567.98f, 700 },
    { 2093.00f, 1500 },
};
constexpr int KARMAN_LEN = sizeof(KARMAN_MELODY) / sizeof(Note);

enum MelodyID { M_NONE, M_PLANET, M_ORBIT, M_KARMAN };

MelodyID currentMelody = M_NONE;
int      currentIdx    = 0;
uint32_t noteStart     = 0;
uint32_t lastEngineUpdate = 0;
bool     engineActive  = false;
bool     karmanPlaying = false;
float    lastAlt       = 0;

static uint8_t vol() {
    int v = (settings.volume * BUZZER_MAX_DUTY) / 100;
    if (v < 0) v = 0;
    if (v > BUZZER_MAX_DUTY) v = BUZZER_MAX_DUTY;
    return (uint8_t)v;
}

static const Note* getMelody(MelodyID id, int& len) {
    switch (id) {
        case M_PLANET: len = PLANET_LEN; return PLANET_MELODY;
        case M_ORBIT:  len = ORBIT_LEN;  return ORBIT_MELODY;
        case M_KARMAN: len = KARMAN_LEN; return KARMAN_MELODY;
        default:       len = 0;          return nullptr;
    }
}

static void startMelody(MelodyID id, uint32_t now) {
    currentMelody = id;
    currentIdx    = 0;
    noteStart     = now;
    int len;
    const Note* m = getMelody(id, len);
    if (m && len > 0) {
        if (m[0].freq > 0.1f) buzzer_tone(m[0].freq, vol());
        else                  buzzer_off();
    }
}

static void stopMelody() {
    currentMelody = M_NONE;
    currentIdx    = 0;
    buzzer_off();
}

void music_init() {
    buzzer_init();
    currentMelody   = M_NONE;
    currentIdx      = 0;
    engineActive    = false;
    karmanPlaying   = false;
    lastAlt         = 0;
    lastEngineUpdate = 0;
}

void music_update(uint32_t now, const Ship& ship, bool onGround) {
    float alt = physics_altitude();

    if (alt < 200.0f) karmanPlaying = false;
    if (!karmanPlaying && lastAlt < 200.0f && alt >= 200.0f) {
        karmanPlaying = true;
        startMelody(M_KARMAN, now);
    }
    lastAlt = alt;

    bool engineNow = ship.throttle > 0.05f;

    if (engineNow) {
        if (!engineActive) {
            engineActive = true;
            stopMelody();
        }
        if (now - lastEngineUpdate >= 30) {
            lastEngineUpdate = now;
            float freq = 200.0f + ship.throttle * 600.0f;
            buzzer_tone(freq, vol());
        }
        return;
    }

    if (engineActive) {
        engineActive = false;
        stopMelody();
    }

    if (currentMelody != M_NONE) {
        int len;
        const Note* m = getMelody(currentMelody, len);
        const Note& n = m[currentIdx];

        if (now - noteStart >= n.durMs) {
            currentIdx++;
            if (currentIdx >= len) {
                if (currentMelody == M_KARMAN) {
                    karmanPlaying = false;
                    stopMelody();
                } else {
                    currentIdx = 0;
                    noteStart  = now;
                    if (m[0].freq > 0.1f) buzzer_tone(m[0].freq, vol());
                    else                  buzzer_off();
                }
            } else {
                noteStart = now;
                if (m[currentIdx].freq > 0.1f) buzzer_tone(m[currentIdx].freq, vol());
                else                           buzzer_off();
            }
        }
        return;
    }

    MelodyID want;
    if (onGround || alt < 200.0f) want = M_PLANET;
    else                          want = M_ORBIT;
    startMelody(want, now);
}