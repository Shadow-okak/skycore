#include "worldmap.h"
#include <math.h>
#include "display.h"
#include "grid.h"
#include "../core/physics.h"
#include "../core/orbit.h"
#include "../settings.h"
#include "../config.h"
#include "../language.h"

void worldmap_draw() {
    U8G2* g = display_get();

    const int cx = 64, cy = 32;
    const float S = 0.12f;

    int bi = physics_nearestBody();
    int bpx = cx + (int)((bodies[bi].x - ship.x) * S);
    int bpy = cy + (int)((bodies[bi].y - ship.y) * S);

    if (settings.grid_mode != GRID_OFF && settings.grid_in_worldmap) {
        int step_px = (int)(settings.grid_step * S);
        if (step_px < 4) step_px = 4;
        if (settings.grid_mode == GRID_CIRCLE)
            grid_draw_circle(g, bpx, bpy, step_px, 0, 0, 127, 63);
        else
            grid_draw_square(g, bpx, bpy, step_px, 0, 0, 127, 63);
    }

    for (int i = 0; i < BODY_COUNT; i++) {
        int px = cx + (int)((bodies[i].x - ship.x) * S);
        int py = cy + (int)((bodies[i].y - ship.y) * S);
        int pr = (int)(bodies[i].radius * S);
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
    }

    float karmanLine = settings.karmanLine * bodies[0].karmanMult;
    if (karmanLine > 0.0f) {
        int p0x = cx + (int)((bodies[0].x - ship.x) * S);
        int p0y = cy + (int)((bodies[0].y - ship.y) * S);
        int kr = (int)((bodies[0].radius + karmanLine) * S);
        for (int a = 0; a < 360; a += 8) {
            float rad = a * 3.14159f / 180.0f;
            int x = p0x + (int)(cosf(rad) * kr);
            int y = p0y + (int)(sinf(rad) * kr);
            if (x < 0 || x > 127 || y < 0 || y > 63) continue;
            g->drawPixel(x, y);
        }
    }

    if (settings.showPrediction) {
        for (int i = 0; i < predictCount; i++) {
            int sx = cx + (int)((predict[i].x - ship.x) * S);
            int sy = cy + (int)((predict[i].y - ship.y) * S);
            if (sx < 0 || sx > 127 || sy < 0 || sy > 63) continue;
            g->drawPixel(sx, sy);
        }
    }

    g->drawDisc(cx, cy, 2);
    int nx = cx + (int)(sinf(ship.angle) * 6);
    int ny = cy - (int)(cosf(ship.angle) * 6);
    g->drawLine(cx, cy, nx, ny);

    g->setFont(u8g2_font_6x12_t_cyrillic);
    g->drawUTF8(2, 10, L_MAP_TITLE);
}