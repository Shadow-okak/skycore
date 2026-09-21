#pragma once
#include <stdint.h>

enum SensorID {
    SENSOR_NONE,
    SENSOR_SPEED,
    SENSOR_VSPEED,
    SENSOR_HSPEED,
    SENSOR_ALTITUDE,
    SENSOR_APOAPSIS,
    SENSOR_PERIAPSIS,
    SENSOR_FUEL,
    SENSOR_THROTTLE,
    SENSOR_TWR,
    SENSOR_ACCEL,
    SENSOR_ANGVEL,
    SENSOR_ANGLE,
    SENSOR_MASS,
    SENSOR_COUNT
};

const char* sensor_label(int id);
void        sensor_format(int id, char* buf, int bufSize);