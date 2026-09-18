#include <Arduino.h>
#include "config.h"
#include "settings.h"
#include "language.h"
#include "input/stick.h"
#include "input/mode.h"
#include "core/physics.h"
#include "core/orbit.h"
#include "core/camera.h"
#include "render/display.h"
#include "render/viewport.h"
#include "render/minimap.h"
#include "render/hud.h"
#include "render/worldmap.h"
#include "menu/menu.h"
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
    settings_init();
    mode_init();
    stick_init();
    stick_calibrate();

    physics_init();
    orbit_clear();
    menu_init();
    music_init();
    debug_init();

    Serial.println(L_BOOT_MESSAGE);
}

void loop() {
    uint32_t now = millis();
    ScreenMode mode = mode_read();

    if (now - lastPhys >= (uint32_t)(DT * 1000)) {
        lastPhys = now;

        Stick in = stick_read();
        if (mode == MODE_MENU) {
            in.x = 0; in.y = 0; in.btn = false;
        }

        physics_updateThrottle(in);

        int speedMult = settings_timeSpeedValue();
        for (int i = 0; i < speedMult; i++) {
            physics_step(in);
        }

        if (now - lastTrail >= 50) {
            lastTrail = now;
            orbit_pushTrail();
        }
        if (now - lastPredict >= 100) {
            lastPredict = now;
            orbit_predict();
        }

        debug_tick(now, ship);
    }

    if (mode == MODE_MENU) {
        if (now - lastRender >= 33) {
            lastRender = now;
            Stick in = stick_read();
            menu_update(now, in);

            display_frameBegin();
            menu_draw();
            display_frameEnd();
        }
        return;
    }

    if (now - lastRender >= 33) {
        lastRender = now;

        camera_update();
        music_update(now, ship, physics_onGround());

        display_frameBegin();
        if (mode == MODE_WORLDMAP) {
            worldmap_draw();
        } else {
            viewport_draw();
            minimap_draw();
            display_get()->drawVLine(63, 0, 64);
            hud_draw();
        }
        display_frameEnd();
    }
}