#include <Arduino.h>
#include "config.h"
#include "input/stick.h"
#include "core/physics.h"
#include "core/orbit.h"
#include "core/camera.h"
#include "render/display.h"
#include "render/viewport.h"
#include "render/minimap.h"
#include "render/hud.h"
#include "audio/music.h"
#include "debug.h"

uint32_t lastPhys    = 0;
uint32_t lastTrail   = 0;
uint32_t lastPredict = 0;
uint32_t lastRender  = 0;

void setup() {
    Serial.begin(115200);
    delay(300);

    display_init();
    stick_init();
    stick_calibrate();

    Serial.printf("Калибровка: cx=%d cy=%d rx=%d ry=%d\n",
                  stick_centerX, stick_centerY,
                  stick_rangeX, stick_rangeY);

    physics_init();
    orbit_clear();
    music_init();
    debug_init();


    Serial.println("Игра запущена. Жду событий...");
}

void loop() {
    uint32_t now = millis();

    // ===== Физика 60 Гц (×SPEED_MULT) =====
    if (now - lastPhys >= (uint32_t)(DT * 1000)) {
        lastPhys = now;
        Stick in = stick_read();

        // Сглаживание газа — один раз за кадр (не зависит от SPEED_MULT)
        physics_updateThrottle(in);

        for (int i = 0; i < SPEED_MULT; i++) {
            physics_step(in);
        }

        // Трейл
        if (now - lastTrail >= 50) {
            lastTrail = now;
            orbit_pushTrail();
        }

        // Предсказание — теперь 10 раз в секунду (было 5)
        if (now - lastPredict >= 100) {
            lastPredict = now;
            orbit_predict();
        }

        // Serial-лог
        debug_tick(now, ship);
    }

    // ===== Рендер 30 FPS =====
    if (now - lastRender >= 33) {
        lastRender = now;

        camera_update();
        music_update(now, ship, physics_onGround());

        display_frameBegin();
        viewport_draw();
        minimap_draw();
        display_get()->drawVLine(63, 0, 64);
        hud_draw();
        display_frameEnd();
    }
}