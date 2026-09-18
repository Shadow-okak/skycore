#include "viewport.h"
#include <math.h>
#include "display.h"
#include "../core/physics.h"
#include "../core/camera.h"
#include "../config.h"

void viewport_draw() {
    U8G2* g = display_get();
    g->setClipWindow(0, 0, 62, 63);

    const int cx = 31, cy = 32;

    float dx = world.planetX - ship.x;
    float dy = world.planetY - ship.y;
    float c = cosf(camRot), s = sinf(camRot);
    float rx = dx * c - dy * s;
    float ry = dx * s + dy * c;
    int px = cx + (int)rx;
    int py = cy + (int)ry;
    int pr = (int)world.planetR;

    int top = py - pr, bot = py + pr;
    if (top < 0)  top = 0;
    if (bot > 63) bot = 63;
    for (int y = top; y <= bot; y++) {
        int dyy = y - py;
        int dx2 = pr*pr - dyy*dyy;
        if (dx2 < 0) continue;
        int dxx = (int)sqrtf((float)dx2);
        int x1 = px - dxx, x2 = px + dxx;
        if (x2 < 0 || x1 > 62) continue;
        if (x1 < 0)  x1 = 0;
        if (x2 > 62) x2 = 62;
        g->drawHLine(x1, y, x2 - x1 + 1);
    }

    float screenAngle = ship.angle + camRot;
    int nx, ny, lbx, lby, rbx, rby;
    display_rotatePoint(0, -8, screenAngle, nx, ny, cx, cy);
    display_rotatePoint(-6, 6, screenAngle, lbx, lby, cx, cy);
    display_rotatePoint(6, 6, screenAngle, rbx, rby, cx, cy);

    g->drawTriangle(nx, ny, lbx, lby, rbx, rby);
    g->drawLine(lbx, lby, rbx, rby);

    if (ship.throttle > 0.05f) {
        int tx, ty;
        display_rotatePoint(0, 6, screenAngle, tx, ty, cx, cy);
        float sTailX = -sinf(screenAngle);
        float sTailY =  cosf(screenAngle);
        int bx = tx + (int)(sTailX * 5);
        int by = ty + (int)(sTailY * 5);
        g->drawLine(tx, ty, bx, by);
    }

    g->setMaxClipWindow();
}