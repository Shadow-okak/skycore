#include "viewport.h"
#include <math.h>
#include "../pins/display.h"
#include "grid.h"
#include "sprites/rocket.h"
#include "../sim/physics.h"
#include "../sim/camera.h"
#include "../settings.h"
#include "../config.h"

static const uint8_t* pickRocketSprite(bool thrusting) {
    bool onGround = physics_onGround();
    if (onGround)  return thrusting ? rocket_boost : rocket_landed;
    if (thrusting) return rocket_launch;
    return rocket_drift;
}

static const uint8_t STARS[][2] = {
    { 5, 18 }, { 15, 25 }, { 48, 17 }, { 55, 40 }, { 8, 55 },
    { 40, 20 }, { 20, 48 }, { 55, 55 }, { 30, 17 }, { 50, 30 },
    { 12, 40 }, { 45, 60 }, { 35, 55 }, { 22, 33 }, { 58, 22 },
};
constexpr int STARS_COUNT = sizeof(STARS) / sizeof(STARS[0]);

void viewport_draw() {
    U8G2* g = display_get();
    g->setClipWindow(0, 12, 62, 63);

    const int cx = 31;
    const int cy = 12 + 26;

    float c = cosf(camRot), s = sinf(camRot);

    for (int i = 0; i < STARS_COUNT; i++) {
        g->drawPixel(STARS[i][0], STARS[i][1]);
    }

    if (settings.grid_mode != GRID_OFF && settings.grid_in_viewport) {
        int bi = physics_nearestBody();
        float bdx = bodies[bi].x - ship.x;
        float bdy = bodies[bi].y - ship.y;
        float brx = bdx * c - bdy * s;
        float bry = bdx * s + bdy * c;
        int bcx = cx + (int)brx;
        int bcy = cy + (int)bry;
        int step_px = settings.grid_step;
        if (settings.grid_mode == GRID_CIRCLE)
            grid_draw_circle(g, bcx, bcy, step_px, 0, 12, 62, 63);
        else
            grid_draw_square(g, bcx, bcy, step_px, 0, 12, 62, 63);
    }

    for (int bi = 0; bi < BODY_COUNT; bi++) {
        float bdx = bodies[bi].x - ship.x;
        float bdy = bodies[bi].y - ship.y;
        float brx = bdx * c - bdy * s;
        float bry = bdx * s + bdy * c;
        int bpx = cx + (int)brx;
        int bpy = cy + (int)bry;
        int bpr = (int)bodies[bi].radius;

        int top = bpy - bpr, bot = bpy + bpr;
        if (top < 12) top = 12;
        if (bot > 63) bot = 63;
        for (int y = top; y <= bot; y++) {
            int dyy = y - bpy;
            int dx2 = bpr*bpr - dyy*dyy;
            if (dx2 < 0) continue;
            int dxx = (int)sqrtf((float)dx2);
            int x1 = bpx - dxx, x2 = bpx + dxx;
            if (x2 < 0 || x1 > 62) continue;
            if (x1 < 0)  x1 = 0;
            if (x2 > 62) x2 = 62;
            g->drawHLine(x1, y, x2 - x1 + 1);
        }
    }

    const int RING_R = 11;
    for (int a = 0; a < 360; a += 30) {
        float rad = a * 3.14159f / 180.0f;
        int x = cx + (int)(cosf(rad) * RING_R);
        int y = cy + (int)(sinf(rad) * RING_R);
        g->drawPixel(x, y);
    }

    float screenAngle = ship.angle + camRot;
    const uint8_t* spr = pickRocketSprite(ship.throttle > 0.05f);
    display_drawSpriteRotated(cx, cy,
                              SPRITE_W, SPRITE_H, spr, screenAngle);

    g->setMaxClipWindow();
}