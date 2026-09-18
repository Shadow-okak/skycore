#include "physics.h"
#include <Arduino.h>
#include <math.h>
#include "../config.h"

World world;
Ship  ship;

void physics_init() {
    world.planetX    = 64;
    world.planetY    = 100;
    world.planetR    = 80;
    world.gravity    = 8.0f;
    world.karmanLow  = 30.0f;
    world.karmanHigh = 90.0f;

    ship.x = 64;
    ship.y = 0;
    ship.vx = 0;
    ship.vy = 0;
    ship.angle = 0;
    ship.fuel = 1.0f;
    ship.throttle = 0;
    ship.throttleTarget = 0;
}

// Плавный отклик газа (вызывается в loop 30 раз в секунду)
void physics_updateThrottle(const Stick& in) {
    float t = in.y;
    if (t < 0.02f) t = 0;
    // S-curve: плавно на малых, насыщается на больших
    t = t * t * (3.0f - 2.0f * t);
    ship.throttleTarget += (t - ship.throttleTarget) * THROTTLE_SMOOTH_K;
    if (ship.throttleTarget < 0.005f) ship.throttleTarget = 0;
    if (ship.throttleTarget > 1.0f)   ship.throttleTarget = 1.0f;
}

void physics_step(const Stick& in) {
    // Поворот
    ship.angle += in.x * 2.5f * DT;
    if (ship.angle >= TWO_PI) ship.angle -= TWO_PI;
    if (ship.angle <  0)      ship.angle += TWO_PI;

    // Газ — используем сглаженное значение (обновляется в loop)
    float throttle = ship.throttleTarget;
    if (throttle > 0 && ship.fuel > 0) {
        ship.throttle = throttle;
        float noseX =  sinf(ship.angle);
        float noseY = -cosf(ship.angle);
        float power = throttle * 50.0f;    // было 100, стало 50
        ship.vx += noseX * power * DT;
        ship.vy += noseY * power * DT;
        ship.fuel -= throttle * 0.15f * DT;
        if (ship.fuel < 0) ship.fuel = 0;
    } else {
        ship.throttle = 0;
    }

    // Гравитация обратно-квадратичная
    float dx = world.planetX - ship.x;
    float dy = world.planetY - ship.y;
    float d  = sqrtf(dx*dx + dy*dy);
    if (d > 1.0f) {
        float g = world.gravity * (world.planetR * world.planetR) / (d * d);
        ship.vx += (dx / d) * g * DT;
        ship.vy += (dy / d) * g * DT;
    }

    ship.x += ship.vx * DT;
    ship.y += ship.vy * DT;

        // Коллизия
    float px = ship.x - world.planetX;
    float py = ship.y - world.planetY;
    float pd = sqrtf(px*px + py*py);
    float collisionR = world.planetR + SHIP_R;
    if (pd < collisionR) {
        if (pd < 0.01f) pd = 0.01f;
        float nx2 = px / pd;
        float ny2 = py / pd;
        ship.x = world.planetX + nx2 * collisionR;
        ship.y = world.planetY + ny2 * collisionR;

        // 1. Гасим радиальную (внутрь планеты)
        float vn = ship.vx * nx2 + ship.vy * ny2;
        if (vn < 0) {
            ship.vx -= vn * nx2;
            ship.vy -= vn * ny2;
        }

        // 2. Гасим тангенциальную (вдоль поверхности) трением
        vn = ship.vx * nx2 + ship.vy * ny2;   // пересчёт после шага 1, vn >= 0
        float vtx = ship.vx - vn * nx2;
        float vty = ship.vy - vn * ny2;
        constexpr float GROUND_FRICTION = 0.9f;   // 0.9 = быстро стоп, 0.98 = медленно
        ship.vx = vn * nx2 + vtx * GROUND_FRICTION;
        ship.vy = vn * ny2 + vty * GROUND_FRICTION;
    }
}

float physics_altitude() {
    float dx = ship.x - world.planetX;
    float dy = ship.y - world.planetY;
    float dist = sqrtf(dx*dx + dy*dy);
    float alt = dist - world.planetR;
    return alt < 0 ? 0 : alt;
}

float physics_localVerticalSpeed() {
    float toPlanetX = world.planetX - ship.x;
    float toPlanetY = world.planetY - ship.y;
    float dl = sqrtf(toPlanetX*toPlanetX + toPlanetY*toPlanetY);
    if (dl < 0.01f) return 0;
    return -(ship.vx * toPlanetX + ship.vy * toPlanetY) / dl;
}

bool physics_onGround() {
    float dx = ship.x - world.planetX;
    float dy = ship.y - world.planetY;
    float dist = sqrtf(dx*dx + dy*dy);
    return dist <= world.planetR + SHIP_R + 0.5f;
}