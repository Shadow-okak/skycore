#include "camera.h"
#include <math.h>
#include "physics.h"

float camRot = 0;

void camera_update() {
    int bi = physics_nearestBody();
    const Body& b = bodies[bi];

    float toBodyX = b.x - ship.x;
    float toBodyY = b.y - ship.y;
    float d = sqrtf(toBodyX*toBodyX + toBodyY*toBodyY);
    if (d < 0.01f) { toBodyX = 0; toBodyY = 1; d = 1; }
    toBodyX /= d;
    toBodyY /= d;

    float alt = d - b.radius;
    float karmanLine = physics_bodyKarman(bi);

    float t;
    if (karmanLine <= 0.0f) {
        t = 1.0f;
    } else if (alt < karmanLine) {
        t = 1.0f;
    } else if (alt < karmanLine * 3.0f) {
        t = (karmanLine * 3.0f - alt) / (karmanLine * 2.0f);
    } else {
        t = 0.0f;
    }

    float lx = -toBodyX, ly = -toBodyY;
    float wx = 0, wy = -1;
    float cux = lx * t + wx * (1 - t);
    float cuy = ly * t + wy * (1 - t);
    float cl = sqrtf(cux*cux + cuy*cuy);
    if (cl > 0.01f) { cux /= cl; cuy /= cl; }

    camRot = atan2f(-cux, -cuy);
}