#include "physics.h"
#include <Arduino.h>
#include <math.h>
#include "../config.h"
#include "../settings.h"
#include "sas.h"

Body bodies[BODY_COUNT];
Ship ship;

void physics_init() {
    // Земля — большая, центр глубоко под экраном
    bodies[0].x = 64;
    bodies[0].y = 250;
    bodies[0].radius = 200;
    bodies[0].gravityMult = 1.0f;
    bodies[0].karmanMult = 1.0f;

    // Луна — в 3.3 раза меньше земли, дальше
    bodies[1].x = 500;
    bodies[1].y = -100;
    bodies[1].radius = 60;
    bodies[1].gravityMult = 0.5f;
    bodies[1].karmanMult = 0.0f;

    ship.x = 64;
    ship.y = 0;
    ship.vx = 0;
    ship.vy = 0;
    ship.angle = 0;
    ship.angularVel = 0;
    ship.accel = 0;
    ship.fuel = 1.0f;
    ship.throttle = 0;
    ship.throttleTarget = 0;
}

float physics_bodyGravity(int i) {
    return settings.gravity * bodies[i].gravityMult;
}

float physics_bodyKarman(int i) {
    return settings.karmanLine * bodies[i].karmanMult;
}

int physics_nearestBody() {
    int best = 0;
    float bestD2 = 1e18f;
    for (int i = 0; i < BODY_COUNT; i++) {
        float dx = bodies[i].x - ship.x;
        float dy = bodies[i].y - ship.y;
        float d2 = dx*dx + dy*dy;
        if (d2 < bestD2) { bestD2 = d2; best = i; }
    }
    return best;
}

float physics_altitude() {
    int i = physics_nearestBody();
    float dx = ship.x - bodies[i].x;
    float dy = ship.y - bodies[i].y;
    float dist = sqrtf(dx*dx + dy*dy);
    float alt = dist - bodies[i].radius;
    return alt < 0 ? 0 : alt;
}

float physics_atmosphere() {
    int bi = physics_nearestBody();
    float karman = physics_bodyKarman(bi);
    if (karman <= 0.0f) return 0.0f;
    float alt = physics_altitude();
    if (alt >= karman) return 0.0f;
    return expf(-alt * 5.0f / karman);
}

float physics_localVerticalSpeed() {
    int i = physics_nearestBody();
    float toX = bodies[i].x - ship.x;
    float toY = bodies[i].y - ship.y;
    float dl = sqrtf(toX*toX + toY*toY);
    if (dl < 0.01f) return 0;
    return -(ship.vx * toX + ship.vy * toY) / dl;
}

bool physics_shipRectCollision(int i, float& nx, float& ny, float& penetration) {
    const Body& b = bodies[i];

    float dx = b.x - ship.x;
    float dy = b.y - ship.y;

    float ca = cosf(ship.angle);
    float sa = sinf(ship.angle);
    float localX =  dx * ca + dy * sa;
    float localY = -dx * sa + dy * ca;

    float closestX = localX;
    float closestY = localY;
    if (closestX >  SHIP_HALF_W) closestX =  SHIP_HALF_W;
    if (closestX < -SHIP_HALF_W) closestX = -SHIP_HALF_W;
    if (closestY >  SHIP_HALF_H) closestY =  SHIP_HALF_H;
    if (closestY < -SHIP_HALF_H) closestY = -SHIP_HALF_H;

    float cx = localX - closestX;
    float cy = localY - closestY;
    float d2 = cx*cx + cy*cy;

    if (d2 >= b.radius * b.radius) return false;

    float d = sqrtf(d2);

    float nLocalX, nLocalY;
    if (d < 0.01f) {
        float dxEdge = SHIP_HALF_W - fabsf(localX);
        float dyEdge = SHIP_HALF_H - fabsf(localY);
        if (dxEdge < dyEdge) {
            nLocalX = (localX > 0) ? -1.0f : 1.0f;
            nLocalY = 0.0f;
            penetration = b.radius + dxEdge;
        } else {
            nLocalX = 0.0f;
            nLocalY = (localY > 0) ? -1.0f : 1.0f;
            penetration = b.radius + dyEdge;
        }
    } else {
        nLocalX = cx / d;
        nLocalY = cy / d;
        penetration = b.radius - d;
    }

    nx = nLocalX * ca - nLocalY * sa;
    ny = nLocalX * sa + nLocalY * ca;

    return true;
}

bool physics_onGroundOf(int i) {
    float dx = bodies[i].x - ship.x;
    float dy = bodies[i].y - ship.y;
    float ca = cosf(ship.angle);
    float sa = sinf(ship.angle);
    float localX =  dx * ca + dy * sa;
    float localY = -dx * sa + dy * ca;

    float closestX = localX;
    float closestY = localY;
    if (closestX >  SHIP_HALF_W) closestX =  SHIP_HALF_W;
    if (closestX < -SHIP_HALF_W) closestX = -SHIP_HALF_W;
    if (closestY >  SHIP_HALF_H) closestY =  SHIP_HALF_H;
    if (closestY < -SHIP_HALF_H) closestY = -SHIP_HALF_H;

    float cx = localX - closestX;
    float cy = localY - closestY;
    float d2 = cx*cx + cy*cy;

    float r = bodies[i].radius + 0.5f;
    return d2 <= r * r;
}

bool physics_onGround() {
    for (int i = 0; i < BODY_COUNT; i++) {
        if (physics_onGroundOf(i)) return true;
    }
    return false;
}

float ship_mass() {
    return MASS_DRY + MASS_FUEL_MAX * ship.fuel;
}

float ship_twr() {
    int bi = physics_nearestBody();
    float g = physics_bodyGravity(bi);
    if (g < 0.01f) return 0.0f;
    float thrust = ship.throttle * (settings.thrust / 50.0f) * THRUST_FORCE;
    return thrust / (ship_mass() * g);
}

void physics_updateThrottle(const Stick& in) {
    float t = in.y;
    if (t < 0.02f) t = 0;
    t = t * t * (3.0f - 2.0f * t);
    ship.throttleTarget += (t - ship.throttleTarget) * THROTTLE_SMOOTH_K;
    if (ship.throttleTarget < 0.005f) ship.throttleTarget = 0;
    if (ship.throttleTarget > 1.0f)   ship.throttleTarget = 1.0f;
}

static void computeGravity(float px, float py, float& ax, float& ay) {
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

void physics_step(const Stick& in) {
    float vxOld = ship.vx;
    float vyOld = ship.vy;

    if (!physics_onGround()) {
        ship.angularVel += in.x * ROT_ACCEL * DT;

        if (settings.sas_enabled) {
            sas_apply(in, DT);
        }

        int bi = physics_nearestBody();
        float karman = physics_bodyKarman(bi);
        float alt = physics_altitude();
        float damp = (karman > 0 && alt < karman) ? ROT_DAMP_ATMO : ROT_DAMP_SPACE;
        ship.angularVel -= ship.angularVel * damp * DT;

        ship.angle += ship.angularVel * DT;
        if (ship.angle >= TWO_PI) ship.angle -= TWO_PI;
        if (ship.angle <  0)      ship.angle += TWO_PI;
    } else {
        ship.angularVel = 0;
    }

    float throttle = ship.throttleTarget;
    if (throttle > 0 && ship.fuel > 0) {
        ship.throttle = throttle;
        float noseX =  sinf(ship.angle);
        float noseY = -cosf(ship.angle);
        float force = throttle * (settings.thrust / 50.0f) * THRUST_FORCE;
        float accel = force / ship_mass();
        ship.vx += noseX * accel * DT;
        ship.vy += noseY * accel * DT;

        float rate = throttle * (settings.fuelRate / 100.0f) * 0.15f;
        ship.fuel -= rate * DT;
        if (ship.fuel < 0) ship.fuel = 0;
    } else {
        ship.throttle = 0;
    }

    float ax, ay;
    computeGravity(ship.x, ship.y, ax, ay);

    ship.x += ship.vx * DT + 0.5f * ax * DT * DT;
    ship.y += ship.vy * DT + 0.5f * ay * DT * DT;

    float atmo = physics_atmosphere();
    if (atmo > 0.001f) {
        float v2 = ship.vx*ship.vx + ship.vy*ship.vy;
        float v = sqrtf(v2);
        if (v > 0.5f) {
            float dragAccel = DRAG_K * atmo * v2;
            ship.vx -= (ship.vx / v) * dragAccel * DT;
            ship.vy -= (ship.vy / v) * dragAccel * DT;
        }
    }

    float ax2, ay2;
    computeGravity(ship.x, ship.y, ax2, ay2);
    ship.vx += 0.5f * (ax + ax2) * DT;
    ship.vy += 0.5f * (ay + ay2) * DT;

    float dvx = ship.vx - vxOld;
    float dvy = ship.vy - vyOld;
    ship.accel = sqrtf(dvx*dvx + dvy*dvy) / DT;

    for (int i = 0; i < BODY_COUNT; i++) {
        float nx, ny, pen;
        if (physics_shipRectCollision(i, nx, ny, pen)) {
            float nOutX = -nx;
            float nOutY = -ny;

            ship.x += nOutX * pen;
            ship.y += nOutY * pen;

            float vn = ship.vx * nOutX + ship.vy * nOutY;
            if (vn < 0) {
                ship.vx -= vn * nOutX;
                ship.vy -= vn * nOutY;
            }

            vn = ship.vx * nOutX + ship.vy * nOutY;
            float vtx = ship.vx - vn * nOutX;
            float vty = ship.vy - vn * nOutY;
            constexpr float GROUND_FRICTION = 0.9f;
            ship.vx = vn * nOutX + vtx * GROUND_FRICTION;
            ship.vy = vn * nOutY + vty * GROUND_FRICTION;
        }
    }
}