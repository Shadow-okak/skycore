#include "debug.h"
#include <Arduino.h>
#include "config.h"
#include "core/physics.h"

static bool wasOnGround   = false;
static bool wasFuelEmpty  = false;
static bool wasThrusting  = false;

void debug_init() {
    Serial.println("\n=== SkyCore ===");
    Serial.printf("Ускорение времени: x%d\n", SPEED_MULT);
    wasOnGround = wasFuelEmpty = wasThrusting = false;
}

static void logEvents(const Ship& ship) {
    bool onGround = physics_onGround();

    if (onGround && !wasOnGround) {
        float v = sqrtf(ship.vx*ship.vx + ship.vy*ship.vy);
        Serial.printf("[EV] Посадка  v=%.1f м/с\n", v);
    }
    if (!onGround && wasOnGround) {
        Serial.println("[EV] Взлёт");
    }
    wasOnGround = onGround;

    bool fuelEmpty = (ship.fuel <= 0);
    if (fuelEmpty && !wasFuelEmpty) Serial.println("[EV] Топливо кончилось");
    wasFuelEmpty = fuelEmpty;

    bool thrusting = ship.throttle > 0.05f;
    if (thrusting && !wasThrusting)
        Serial.printf("[EV] Тяга включена  %.0f%%\n", ship.throttle * 100);
    if (!thrusting && wasThrusting)
        Serial.println("[EV] Тяга выключена");
    wasThrusting = thrusting;
}

static void logStatus(const Ship& ship) {
    float alt = physics_altitude();
    float speed = sqrtf(ship.vx*ship.vx + ship.vy*ship.vy);
    float vLocal = physics_localVerticalSpeed();

    Serial.printf("[ST] alt=%.0fм  v=%.1f  vл=%.1f  угол=%.0f°  топ=%d%%\n",
                  alt, speed, vLocal,
                  ship.angle * 57.2958f,
                  (int)(ship.fuel * 100));
}

void debug_tick(uint32_t now, const Ship& ship) {
    logEvents(ship);

    static uint32_t lastStatus = 0;
    if (now - lastStatus >= 1000) {
        lastStatus = now;
        logStatus(ship);
    }
}