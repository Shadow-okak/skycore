#include "minimap.h"
#include <math.h>
#include "display.h"
#include "grid.h"
#include "../core/physics.h"
#include "../core/orbit.h"
#include "../settings.h"
#include "../config.h"

void minimap_draw() {
    U8G2* g = display_get();
    g->setClipWindow(64, 12, 127, 63);

    const int cx = 96;
    const int cy = 12 + 22;
    const float S = 0.10f;

    int bi = physics_nearestBody();
    int bpx = cx + (int)((bodies[bi].x - ship.x) * S);
    int bpy = cy + (int)((bodies[bi].y - ship.y) * S);

    if (settings.grid_mode != GRID_OFF && settings.grid_in_minimap) {
        int step_px = (int)(settings.grid_step * S);
        if (step_px < 4) step_px = 4;
        if (settings.grid_mode == GRID_CIRCLE)
            grid_draw_circle(g, bpx, bpy, step_px, 64, 12, 127, 63);
        else
            grid_draw_square(g, bpx, bpy, step_px, 64, 12, 127, 63);
    }

    for (int i = 0; i < BODY_COUNT; i++) {
        int px = cx + (int)((bodies[i].x - ship.x) * S);
        int py = cy + (int)((bodies[i].y - ship.y) * S);
        int pr = (int)(bodies[i].radius * S);
        if (pr < 3) pr = 3;
        g->drawDisc(px, py, pr);
    }

    float karmanLine = settings.karmanLine * bodies[0].karmanMult;
    if (karmanLine > 0.0f) {
        int p0x = cx + (int)((bodies[0].x - ship.x) * S);
        int p0y = cy + (int)((bodies[0].y - ship.y) * S);
        int kr = (int)((bodies[0].radius + karmanLine) * S);
        for (int a = 0; a < 360; a += 12) {
            float rad = a * 3.14159f / 180.0f;
            int x = p0x + (int)(cosf(rad) * kr);
            int y = p0y + (int)(sinf(rad) * kr);
            if (x < 65 || x > 126 || y < 13 || y > 62) continue;
            g->drawPixel(x, y);
        }
    }

    if (settings.showPrediction) {
        for (int i = 0; i < predictCount; i++) {
            int sx = cx + (int)((predict[i].x - ship.x) * S);
            int sy = cy + (int)((predict[i].y - ship.y) * S);
            if (sx < 65 || sx > 126 || sy < 13 || sy > 62) continue;
            g->drawPixel(sx, sy);
        }
    }

    g->drawDisc(cx, cy, 2);
    int noseX = cx + (int)(sinf(ship.angle) * 5);
    int noseY = cy - (int)(cosf(ship.angle) * 5);
    g->drawLine(cx, cy, noseX, noseY);

    g->setMaxClipWindow();
}