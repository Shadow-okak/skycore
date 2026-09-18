#include "settings.h"

Settings settings;

void settings_init() {
    settings.sas_holdAngle   = false;
    settings.volume          = 70;
    settings.showTrail       = true;
    settings.showPrediction  = true;
    settings.timeSpeed       = 2;
    settings.gravity         = 8.0f;
    settings.thrust          = 50;
    settings.fuelRate        = 15;
    settings.karmanLine      = 30;
}

int settings_timeSpeedValue() {
    static const int v[] = {1, 2, 3, 5};
    int i = settings.timeSpeed;
    if (i < 0) i = 0;
    if (i > 3) i = 3;
    return v[i];
}