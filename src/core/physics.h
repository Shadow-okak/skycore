#pragma once
#include "../input/stick.h"

constexpr int BODY_COUNT = 2;

struct Body {
    float x, y;
    float radius;
    float gravityMult;
    float karmanMult;
};

struct Ship {
    float x, y;
    float vx, vy;
    float angle;
    float angularVel;
    float accel;
    float fuel;
    float throttle;
    float throttleTarget;
};

extern Body bodies[BODY_COUNT];
extern Ship ship;

// Полугабариты корабля в локальной системе (нос = -Y)
constexpr float SHIP_HALF_W = 4.0f;    // половина ширины
constexpr float SHIP_HALF_H = 14.0f;   // половина длины

void  physics_init();
void  physics_updateThrottle(const Stick& in);
void  physics_step(const Stick& in);

int   physics_nearestBody();
float physics_bodyGravity(int i);
float physics_bodyKarman(int i);
float physics_altitude();
float physics_localVerticalSpeed();
float physics_atmosphere();
bool  physics_onGround();
bool  physics_onGroundOf(int i);

// Коллизия корабля (прямоугольник) с телом (круг).
// Возвращает true при пересечении.
// nx, ny — нормаль от корабля к телу (мировые координаты).
// penetration — глубина проникновения.
bool physics_shipRectCollision(int i, float& nx, float& ny, float& penetration);

float ship_mass();
float ship_twr();