#include "sas.h"
#include <Arduino.h>
#include <math.h>
#include "../config.h"
#include "../settings.h"
#include "physics.h"

void sas_apply(const Stick& in, float dt) {
    if (fabs(in.x) > 0.1f) return;

    float targetAngle = ship.angle;
    bool hasTarget = false;
    float v = sqrtf(ship.vx*ship.vx + ship.vy*ship.vy);

    switch (settings.sas_mode) {
        case SAS_PROGRADE:
            if (v > 0.5f) {
                targetAngle = atan2f(ship.vx, -ship.vy);
                hasTarget = true;
            }
            break;
        case SAS_RETROGRADE:
            if (v > 0.5f) {
                targetAngle = atan2f(-ship.vx, ship.vy);
                hasTarget = true;
            }
            break;
        case SAS_HOLD_ANGLE:
            targetAngle = settings.sas_target_angle;
            hasTarget = true;
            break;
    }

    if (!hasTarget) return;

    float error = targetAngle - ship.angle;
    while (error >  PI) error -= TWO_PI;
    while (error < -PI) error += TWO_PI;
    float control = SAS_KP * error - SAS_KD * ship.angularVel;
    ship.angularVel += control * dt;
}