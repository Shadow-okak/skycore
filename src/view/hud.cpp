#include "hud.h"
#include <stdio.h>
#include "../pins/display.h"
#include "../sim/physics.h"
#include "../sim/sensors.h"
#include "../settings.h"
#include "../config.h"
#include "../language.h"

// ===== Нижний правый угол — шкала топлива =====
void hud_draw_fuel() {
    U8G2* g = display_get();
    g->setFont(u8g2_font_6x12_t_cyrillic);
    g->setDrawColor(1);

    g->drawFrame(102, 54, 24, 8);
    int fill = (int)(22.0f * ship.fuel);
    if (fill > 0) g->drawBox(103, 55, fill, 6);
}

// ===== Верхняя полоса — 3 датчика =====
void hud_draw_sensors() {
    U8G2* g = display_get();

    g->drawHLine(0, 11, 128);

    for (int slot = 0; slot < 3; slot++) {
        int x0 = slot * 43;
        int x1 = x0 + 42;

        int id = settings.sensorSlot[slot];
        if (id <= 0 || id >= SENSOR_COUNT) continue;

        const char* label = sensor_label(id);
        char val[16];
        sensor_format(id, val, sizeof(val));

        g->setFont(u8g2_font_5x7_t_cyrillic);
        g->drawUTF8(x0 + 2, 9, label);

        g->setFont(u8g2_font_6x12_t_cyrillic);
        g->drawUTF8(x0 + 14, 10, val);

        if (slot < 2) {
            g->drawVLine(x1, 0, 11);
        }
    }

    g->setFont(u8g2_font_6x12_t_cyrillic);

    if (settings.sas_enabled) {
        g->drawBox(124, 1, 3, 3);
    }
}