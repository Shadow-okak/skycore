#include "minimap.h"
#include <math.h>
#include "display.h"
#include "../core/physics.h"
#include "../core/orbit.h"
#include "../config.h"

void minimap_draw() {
    U8G2* g = display_get();
    g->setClipWindow(64, 0, 127, 63);

    const int cx = 96;
    const int cy = 34;
    const float S = 0.10f;

    // Планета
    int pr = (int)(world.planetR * S);
    if (pr < 4) pr = 4;
    g->drawDisc(cx, cy, pr);

    // Линия Кармана
    int kr = (int)((world.planetR + world.karmanLow) * S);
    for (int a = 0; a < 360; a += 12) {
        float rad = a * 3.14159f / 180.0f;
        int x = cx + (int)(cosf(rad) * kr);
        int y = cy + (int)(sinf(rad) * kr);
        if (x < 65 || x > 126) continue;
        if (y < 2 || y > 62) continue;
        g->drawPixel(x, y);
        g->drawPixel(x + 1, y);
    }

    // Предсказание орбиты
    for (int i = 0; i < predictCount; i++) {
        int sx = cx + (int)(predict[i].dx * S);
        int sy = cy + (int)(predict[i].dy * S);
        if (sx < 65 || sx > 126) continue;
        if (sy < 2 || sy > 62) continue;
        g->drawPixel(sx, sy);
    }

    // Трейл прошлого
    for (int i = 0; i < trailCount; i++) {
        int idx = (trailHead - trailCount + i + TRAIL_MAX) % TRAIL_MAX;
        int sx = cx + (int)(trail[idx].dx * S);
        int sy = cy + (int)(trail[idx].dy * S);
        if (sx < 65 || sx > 126) continue;
        if (sy < 2 || sy > 62) continue;
        if (i % 2 == 0) g->drawPixel(sx, sy);
    }

    // Корабль
    float dx = ship.x - world.planetX;
    float dy = ship.y - world.planetY;
    int sx = cx + (int)(dx * S);
    int sy = cy + (int)(dy * S);

    if (sx < 66)  sx = 66;
    if (sx > 125) sx = 125;
    if (sy < 14)  sy = 14;
    if (sy > 52)  sy = 52;

    g->drawDisc(sx, sy, 2);
    int noseX = sx + (int)(sinf(ship.angle) * 5);
    int noseY = sy - (int)(cosf(ship.angle) * 5);
    g->drawLine(sx, sy, noseX, noseY);

    g->setMaxClipWindow();
}