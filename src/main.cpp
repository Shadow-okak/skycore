#include <Arduino.h>
#include "config.h"
#include "settings.h"
#include "language.h"
#include "pins/stick.h"
#include "pins/switches.h"
#include "pins/display.h"
#include "pins/buzzer.h"
#include "sim/physics.h"
#include "sim/orbit.h"
#include "sim/camera.h"
#include "view/viewport.h"
#include "view/minimap.h"
#include "view/hud.h"
#include "view/worldmap.h"
#include "view/menu.h"
#include "audio/music.h"
#include "debug.h"

uint32_t lastPhys    = 0;
uint32_t lastPredict = 0;
uint32_t lastRender  = 0;

void setup() {
    Serial.begin(115200);
    delay(300);

    display_init();
    settings_init();
    switches_init();
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
    ScreenMode mode = switches_read();

    static ScreenMode lastMode = MODE_FLIGHT;
    if (mode != lastMode && mode == MODE_MENU) {
        buzzer_off();
    }
    lastMode = mode;

    if (now - lastPhys >= (uint32_t)(DT * 1000)) {
        lastPhys = now;

        Stick in = stick_read();
        if (mode == MODE_MENU) {
            in.x = 0; in.y = 0; in.btn = false;
        }

        static bool lastBtnSas = false;
        if (mode == MODE_FLIGHT) {
            if (in.btn && !lastBtnSas) {
                settings.sas_enabled = !settings.sas_enabled;
                if (settings.sas_enabled && settings.sas_mode == SAS_HOLD_ANGLE) {
                    settings.sas_target_angle = ship.angle;
                }
            }
            lastBtnSas = in.btn;
        }

        physics_updateThrottle(in);

        int speedMult = settings_timeSpeedValue();
        for (int i = 0; i < speedMult; i++) {
            physics_step(in);
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
            display_get()->drawVLine(63, 12, 52);
            hud_draw_fuel();
            hud_draw_sensors();
        }
        display_frameEnd();
    }
}