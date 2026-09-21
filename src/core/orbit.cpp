#include "orbit.h"
#include <math.h>
#include "../config.h"
#include "physics.h"

TrailPoint predict[PREDICT_MAX];
int predictCount = 0;

void orbit_clear() {
    predictCount = 0;
}

static void predictGravity(float px, float py, float& ax, float& ay) {
    ax = 0; ay = 0;
    for (int i = 0; i < BODY_COUNT; i++) {
        float dx = bodies[i].x - px;
        float dy = bodies[i].y - py;
        float d2 = dx*dx + dy*dy;
        float d = sqrtf(d2);
        if (d > 1.0f) {
            float g0 = physics_bodyGravity(i);
            float g = g0 * (bodies[i].radius * bodies[i].radius) / d2;
            ax += (dx / d) * g;
            ay += (dy / d) * g;
        }
    }
}

void orbit_predict() {
    float px = ship.x, py = ship.y;
    float pvx = ship.vx, pvy = ship.vy;
    predictCount = 0;

    for (int i = 0; i < PREDICT_STEPS && predictCount < PREDICT_MAX; i++) {
        float ax, ay;
        predictGravity(px, py, ax, ay);
        px += pvx * DT + 0.5f * ax * DT * DT;
        py += pvy * DT + 0.5f * ay * DT * DT;
        float ax2, ay2;
        predictGravity(px, py, ax2, ay2);
        pvx += 0.5f * (ax + ax2) * DT;
        pvy += 0.5f * (ay + ay2) * DT;

        if (i % PREDICT_SKIP == 0) {
            predict[predictCount].x = px;
            predict[predictCount].y = py;
            predictCount++;
        }

        for (int j = 0; j < BODY_COUNT; j++) {
            float ddx = px - bodies[j].x;
            float ddy = py - bodies[j].y;
            if (ddx*ddx + ddy*ddy < bodies[j].radius * bodies[j].radius) {
                i = PREDICT_STEPS;
                break;
            }
        }
    }
}