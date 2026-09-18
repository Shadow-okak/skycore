#include "camera.h"
#include <math.h>
#include "physics.h"

float camRot = 0;

void camera_update() {
    float toPlanetX = world.planetX - ship.x;
    float toPlanetY = world.planetY - ship.y;
    float d = sqrtf(toPlanetX*toPlanetX + toPlanetY*toPlanetY);
    if (d < 0.01f) { toPlanetX = 0; toPlanetY = 1; d = 1; }
    toPlanetX /= d;
    toPlanetY /= d;

    float alt = d - world.planetR;
    float t = 0;
    if (alt < world.karmanLow) t = 1;
    else if (alt < world.karmanHigh)
        t = (world.karmanHigh - alt) / (world.karmanHigh - world.karmanLow);

    float lx = -toPlanetX, ly = -toPlanetY;
    float wx = 0, wy = -1;
    float cux = lx * t + wx * (1 - t);
    float cuy = ly * t + wy * (1 - t);
    float cl = sqrtf(cux*cux + cuy*cuy);
    if (cl > 0.01f) { cux /= cl; cuy /= cl; }

    camRot = atan2f(-cux, -cuy);
}