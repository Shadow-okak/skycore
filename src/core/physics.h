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
    float throttle;
    float throttleTarget;
};

extern World world;
extern Ship  ship;

constexpr float SHIP_R = 5.0f;

void  physics_init();
void  physics_updateThrottle(const Stick& in);
void  physics_step(const Stick& in);

float physics_altitude();
float physics_localVerticalSpeed();
bool  physics_onGround();