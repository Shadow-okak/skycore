#include "music.h"
#include <Arduino.h>
#include <math.h>
#include "buzzer.h"
#include "../config.h"
#include "../core/physics.h"

// ===== Формат ноты =====
struct Note {
    float freq;      // Гц, 0 = пауза
    uint32_t durMs;
};

// ===== МЕЛОДИЯ ПЛАНЕТЫ =====
// Тёплая минорная пентатоника, медленно. Играет на поверхности и в атмосфере.
const Note PLANET_MELODY[] = {
    { 220.00f, 600 },  // A3
    {    0.0f, 100 },
    { 261.63f, 500 },  // C4
    { 293.66f, 500 },  // D4
    {    0.0f, 200 },
    { 246.94f, 500 },  // B3
    { 220.00f, 700 },  // A3
    { 196.00f, 500 },  // G3
    {    0.0f, 800 },
};
constexpr int PLANET_LEN = sizeof(PLANET_MELODY) / sizeof(Note);

// ===== ОРБИТАЛЬНАЯ МЕЛОДИЯ =====
// Редкие высокие ноты, эмбиент, длинные паузы.
const Note ORBIT_MELODY[] = {
    { 523.25f, 400 },  // C5
    {    0.0f, 400 },
    { 659.25f, 400 },  // E5
    {    0.0f, 600 },
    { 587.33f, 400 },  // D5
    {    0.0f, 600 },
    { 783.99f, 600 },  // G5
    {    0.0f, 1000 },
};
constexpr int ORBIT_LEN = sizeof(ORBIT_MELODY) / sizeof(Note);

// ===== ВЫХОД ИЗ КАРМАНА =====
// Восходящее мажорное арпеджио, ~5 секунд.
const Note KARMAN_MELODY[] = {
    {  261.63f, 300 },  // C4
    {  329.63f, 300 },  // E4
    {  392.00f, 300 },  // G4
    {  523.25f, 300 },  // C5
    {  659.25f, 300 },  // E5
    {  783.99f, 500 },  // G5
    { 1046.50f, 500 },  // C6
    { 1318.51f, 500 },  // E6
    { 1567.98f, 700 },  // G6
    { 2093.00f, 1500 }, // C7 — финальный аккорд
};
constexpr int KARMAN_LEN = sizeof(KARMAN_MELODY) / sizeof(Note);

// ===== Состояние =====
enum MelodyID { M_NONE, M_PLANET, M_ORBIT, M_KARMAN };

MelodyID currentMelody = M_NONE;
int      currentIdx    = 0;
uint32_t noteStart     = 0;
uint32_t lastEngineUpdate = 0;
bool     engineActive  = false;
bool     karmanPlaying = false;
float    lastAlt       = 0;

// ===== Внутренние =====
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
    if (m && len > 0) buzzer_tone(m[0].freq);
}

static void stopMelody() {
    currentMelody = M_NONE;
    currentIdx    = 0;
    buzzer_off();
}

// ===== Публичные =====
void music_init() {
    buzzer_init();
    currentMelody   = M_NONE;
    currentIdx      = 0;
    engineActive    = false;
    karmanPlaying   = false;
    lastAlt         = 0;
    lastEngineUpdate = 0;
}

void music_playStartup() {
    // Не используется — фоновая мелодия включается автоматически
}

void music_update(uint32_t now, const Ship& ship, bool onGround) {
    float alt = physics_altitude();

    // ===== Событие: выход из линии Кармана =====
    if (alt < world.karmanHigh) {
        karmanPlaying = false;    // вернулись — можно услышать снова
    }
    if (!karmanPlaying && lastAlt < world.karmanHigh && alt >= world.karmanHigh) {
        karmanPlaying = true;
        startMelody(M_KARMAN, now);
    }
    lastAlt = alt;

    // ===== Двигатель (приоритет выше всего) =====
    bool engineNow = ship.throttle > 0.05f;

    if (engineNow) {
        if (!engineActive) {
            engineActive = true;
            stopMelody();       // глушим фон
        }
        if (now - lastEngineUpdate >= 30) {
            lastEngineUpdate = now;
            float freq = 200.0f + ship.throttle * 600.0f;
            buzzer_tone(freq);
        }
        return;
    }

    if (engineActive) {
        // Только что отпустил газ — фон начнётся заново
        engineActive = false;
        stopMelody();
    }

    // ===== Плеер мелодии =====
    if (currentMelody != M_NONE) {
        int len;
        const Note* m = getMelody(currentMelody, len);
        const Note& n = m[currentIdx];

        if (now - noteStart >= n.durMs) {
            currentIdx++;
            if (currentIdx >= len) {
                if (currentMelody == M_KARMAN) {
                    // Эпик не зацикливаем
                    karmanPlaying = false;
                    stopMelody();
                } else {
                    // Планета / Орбита — зацикливаем
                    currentIdx = 0;
                    noteStart  = now;
                    buzzer_tone(m[0].freq);
                }
            } else {
                noteStart = now;
                buzzer_tone(m[currentIdx].freq);
            }
        }
        return;
    }

    // ===== Фон: выбрать, что играть =====
    MelodyID want;
    if (onGround || alt < world.karmanHigh) {
        want = M_PLANET;
    } else {
        want = M_ORBIT;
    }
    startMelody(want, now);
}