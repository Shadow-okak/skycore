#pragma once
#include <stdint.h>

struct Settings {
    // САС
    bool sas_holdAngle;

    // Осн
    int  volume;
    bool showTrail;
    bool showPrediction;
    int  timeSpeed;

    // Физ
    float gravity;
    int   thrust;
    int   fuelRate;
    int   karmanLine;
};

extern Settings settings;

void settings_init();
int  settings_timeSpeedValue();