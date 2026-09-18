#include "worldmap.h"
#include <math.h>
#include "display.h"
#include "../core/physics.h"
#include "../core/orbit.h"
#include "../config.h"
#include "../language.h"

void worldmap_draw() {
    U8G2* g = display_get();

    const int cx = 64, cy = 32;
    const float S = 0.12f;

    int px = cx + (int)((world.planetX - ship.x) * S);
    int py = cy + (int)((world.planetY - ship.y) * S);
    int pr = (int)(world.planetR * S);
    if (pr < 2) pr = 2;
    for (int y = py - pr; y <= py + pr; y++) {
        if (y < 0 || y > 63) continue;
        int dy = y - py;
        int dx2 = pr*pr - dy*dy;
        if (dx2 < 0) continue;
        int dx = (int)sqrtf((float)dx2);
        int x1 = px - dx, x2 = px + dx;
        if (x2 < 0 || x1 > 127) continue;
        if (x1 < 0)   x1 = 0;
        if (x2 > 127) x2 = 127;
        g->drawHLine(x1, y, x2 - x1 + 1);
    }

    int kr = (int)((world.planetR + world.karmanLow) * S);
    for (int a = 0; a < 360; a += 8) {
        float rad = a * 3.14159f / 180.0f;
        int x = px + (int)(cosf(rad) * kr);
        int y = py + (int)(sinf(rad) * kr);
        if (x < 0 || x > 127 || y < 0 || y > 63) continue;
        g->drawPixel(x, y);
    }

    for (int i = 0; i < trailCount; i++) {
        int idx = (trailHead - trailCount + i + TRAIL_MAX) % TRAIL_MAX;
        int sx = cx + (int)((trail[idx].dx - (ship.x - world.planetX)) * S);
        int sy = cy + (int)((trail[idx].dy - (ship.y - world.planetY)) * S);
        if (sx < 0 || sx > 127 || sy < 0 || sy > 63) continue;
        if (i % 2 == 0) g->drawPixel(sx, sy);
    }

    for (int i = 0; i < predictCount; i++) {
        int sx = cx + (int)((predict[i].dx - (ship.x - world.planetX)) * S);
        int sy = cy + (int)((predict[i].dy - (ship.y - world.planetY)) * S);
        if (sx < 0 || sx > 127 || sy < 0 || sy > 63) continue;
        g->drawPixel(sx, sy);
    }

    float a = ship.angle;
    int nx = cx + (int)(sinf(a) * 6);
    int ny = cy - (int)(cosf(a) * 6);
    g->drawDisc(cx, cy, 2);
    g->drawLine(cx, cy, nx, ny);

    g->setFont(u8g2_font_6x12_t_cyrillic);
    g->setDrawColor(1);
    g->drawUTF8(2, 10, L_MAP_TITLE);
    g->drawUTF8(2, 62, L_MAP_ALTITUDE);
    g->print(" ");
    g->print((int)physics_altitude());
    g->print(" ");
    g->drawUTF8(g->getCursorX(), 62, L_HUD_METERS);
}