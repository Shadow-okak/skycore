#include "sensor_bar.h"
#include "display.h"
#include "../settings.h"
#include "../core/sensors.h"
#include "../config.h"
#include "../language.h"
#include <stdio.h>

void sensor_bar_draw() {
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