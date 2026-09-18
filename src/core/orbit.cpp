#include "orbit.h"
#include <math.h>
#include "../config.h"
#include "physics.h"

TrailPoint trail[TRAIL_MAX];
int trailHead = 0;
int trailCount = 0;

TrailPoint predict[PREDICT_MAX];
int predictCount = 0;

void orbit_pushTrail() {
    trail[trailHead].dx = ship.x - world.planetX;
    trail[trailHead].dy = ship.y - world.planetY;
    trailHead = (trailHead + 1) % TRAIL_MAX;
    if (trailCount < TRAIL_MAX) trailCount++;
}

void orbit_clear() {
    trailHead = 0;
    trailCount = 0;
    predictCount = 0;
}

void orbit_predict() {
    float px = ship.x, py = ship.y;
    float pvx = ship.vx, pvy = ship.vy;
    predictCount = 0;

    for (int i = 0; i < PREDICT_STEPS && predictCount < PREDICT_MAX; i++) {
        float dx = world.planetX - px;
        float dy = world.planetY - py;
        float d  = sqrtf(dx*dx + dy*dy);
        if (d > 1.0f) {
            float g = world.gravity * (world.planetR * world.planetR) / (d * d);
            pvx += (dx / d) * g * DT;
            pvy += (dy / d) * g * DT;
        }
        px += pvx * DT;
        py += pvy * DT;

        if (i % PREDICT_SKIP == 0) {
            predict[predictCount].dx = px - world.planetX;
            predict[predictCount].dy = py - world.planetY;
            predictCount++;
        }

        float ddx = px - world.planetX;
        float ddy = py - world.planetY;
        if (ddx*ddx + ddy*ddy < world.planetR * world.planetR) break;
    }
}