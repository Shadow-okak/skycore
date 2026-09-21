#include "sensors.h"
#include <Arduino.h>
#include <math.h>
#include <stdio.h>
#include "../config.h"
#include "../language.h"
#include "../settings.h"
#include "physics.h"
#include "orbit.h"

static void format_number(float v, char* buf, int size) {
    float av = fabsf(v);
    if (av >= 1000000.0f)    snprintf(buf, size, "%.1fМ", v / 1000000.0f);
    else if (av >= 10000.0f) snprintf(buf, size, "%.0fк", v / 1000.0f);
    else if (av >= 1000.0f)  snprintf(buf, size, "%.1fк", v / 1000.0f);
    else if (av >= 100.0f)   snprintf(buf, size, "%.0f", v);
    else if (av >= 10.0f)    snprintf(buf, size, "%.1f", v);
    else                     snprintf(buf, size, "%.2f", v);
}

const char* sensor_label(int id) {
    switch (id) {
        case SENSOR_NONE:      return L_SENSOR_NONE;
        case SENSOR_SPEED:     return L_SENSOR_SPEED;
        case SENSOR_VSPEED:    return L_SENSOR_VSPEED;
        case SENSOR_HSPEED:    return L_SENSOR_HSPEED;
        case SENSOR_ALTITUDE:  return L_SENSOR_ALTITUDE;
        case SENSOR_APOAPSIS:  return L_SENSOR_APOAPSIS;
        case SENSOR_PERIAPSIS: return L_SENSOR_PERIAPSIS;
        case SENSOR_FUEL:      return L_SENSOR_FUEL;
        case SENSOR_THROTTLE:  return L_SENSOR_THROTTLE;
        case SENSOR_TWR:       return L_SENSOR_TWR;
        case SENSOR_ACCEL:     return L_SENSOR_ACCEL;
        case SENSOR_ANGVEL:    return L_SENSOR_ANGVEL;
        case SENSOR_ANGLE:     return L_SENSOR_ANGLE;
        case SENSOR_MASS:      return L_SENSOR_MASS;
        default:               return "??";
    }
}

static void calc_orbit_extremes(float& apo, float& peri) {
    apo = 0.0f;
    peri = 1e18f;
    if (predictCount == 0) { apo = -1; peri = -1; return; }
    for (int i = 0; i < predictCount; i++) {
        float bestD = 1e18f;
        for (int j = 0; j < BODY_COUNT; j++) {
            float dx = predict[i].x - bodies[j].x;
            float dy = predict[i].y - bodies[j].y;
            float d = sqrtf(dx*dx + dy*dy) - bodies[j].radius;
            if (d < bestD) bestD = d;
        }
        if (bestD > apo) apo = bestD;
        if (bestD < peri) peri = bestD;
    }
    if (peri < 0) peri = 0;
}

void sensor_format(int id, char* buf, int bufSize) {
    switch (id) {
        case SENSOR_NONE:
            snprintf(buf, bufSize, "---");
            break;
        case SENSOR_SPEED: {
            float v = sqrtf(ship.vx*ship.vx + ship.vy*ship.vy);
            format_number(v, buf, bufSize);
            break;
        }
        case SENSOR_VSPEED:
            format_number(physics_localVerticalSpeed(), buf, bufSize);
            break;
        case SENSOR_HSPEED: {
            int bi = physics_nearestBody();
            float toX = bodies[bi].x - ship.x;
            float toY = bodies[bi].y - ship.y;
            float dl = sqrtf(toX*toX + toY*toY);
            if (dl < 0.01f) { snprintf(buf, bufSize, "0"); break; }
            float px = -toY / dl;
            float py =  toX / dl;
            float v = ship.vx * px + ship.vy * py;
            format_number(v, buf, bufSize);
            break;
        }
        case SENSOR_ALTITUDE:
            format_number(physics_altitude(), buf, bufSize);
            break;
        case SENSOR_APOAPSIS: {
            float apo, peri;
            calc_orbit_extremes(apo, peri);
            if (apo < 0) { snprintf(buf, bufSize, "---"); break; }
            format_number(apo, buf, bufSize);
            break;
        }
        case SENSOR_PERIAPSIS: {
            float apo, peri;
            calc_orbit_extremes(apo, peri);
            if (peri < 0) { snprintf(buf, bufSize, "---"); break; }
            format_number(peri, buf, bufSize);
            break;
        }
        case SENSOR_FUEL:
            snprintf(buf, bufSize, "%d%%", (int)(ship.fuel * 100));
            break;
        case SENSOR_THROTTLE:
            snprintf(buf, bufSize, "%d%%", (int)(ship.throttle * 100));
            break;
        case SENSOR_TWR: {
            float t = ship_twr();
            if (t < 0.01f) { snprintf(buf, bufSize, "---"); break; }
            snprintf(buf, bufSize, "%.2f", t);
            break;
        }
        case SENSOR_ACCEL:
            format_number(ship.accel, buf, bufSize);
            break;
        case SENSOR_ANGVEL: {
            float v = ship.angularVel * 57.2958f;
            format_number(v, buf, bufSize);
            break;
        }
        case SENSOR_ANGLE:
            snprintf(buf, bufSize, "%.0f", ship.angle * 57.2958f);
            break;
        case SENSOR_MASS:
            format_number(ship_mass(), buf, bufSize);
            break;
        default:
            snprintf(buf, bufSize, "?");
            break;
    }
}