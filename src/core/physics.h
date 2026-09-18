#pragma once
#include "../input/stick.h"

struct World {
    float planetX, planetY;
    float planetR;
    float gravity;
    float karmanLow;
    float karmanHigh;
};

struct Ship {
    float x, y;
    float vx, vy;
    float angle;
    float fuel;
    float throttle;        // текущая тяга (сглаженная)
    float throttleTarget;  // цель от стика
};

extern World world;
extern Ship  ship;

constexpr float SHIP_R = 5.0f;

void  physics_init();
void  physics_updateThrottle(const Stick& in);  // сглаживание газа, 1 раз за кадр
void  physics_step(const Stick& in);            // 1 шаг физики

float physics_altitude();
float physics_localVerticalSpeed();
bool  physics_onGround();