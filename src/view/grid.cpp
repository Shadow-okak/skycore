#include "grid.h"
#include <math.h>

void grid_draw_circle(U8G2* g, int cx, int cy, int step_px,
                      int x0, int y0, int x1, int y1) {
    if (step_px < 4 || step_px > 100) return;

    int maxR = (x1 - x0) + (y1 - y0);
    if (maxR > 200) maxR = 200;

    for (int r = step_px; r < maxR; r += step_px) {
        for (int a = 0; a < 360; a += 6) {
            float rad = a * 3.14159f / 180.0f;
            int x = cx + (int)(cosf(rad) * r);
            int y = cy + (int)(sinf(rad) * r);
            if (x < x0 || x > x1 || y < y0 || y > y1) continue;
            g->drawPixel(x, y);
        }
    }
}

void grid_draw_square(U8G2* g, int cx, int cy, int step_px,
                      int x0, int y0, int x1, int y1) {
    if (step_px < 4 || step_px > 100) return;

    int startX = ((x0 - cx) / step_px) * step_px + cx;
    for (int x = startX; x <= x1; x += step_px) {
        if (x < x0) continue;
        g->drawVLine(x, y0, y1 - y0 + 1);
    }
    int startY = ((y0 - cy) / step_px) * step_px + cy;
    for (int y = startY; y <= y1; y += step_px) {
        if (y < y0) continue;
        g->drawHLine(x0, y, x1 - x0 + 1);
    }
}