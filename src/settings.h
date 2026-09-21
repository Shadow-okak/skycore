#pragma once
#include <stdint.h>

enum SasMode {
    SAS_PROGRADE,
    SAS_RETROGRADE,
    SAS_HOLD_ANGLE,
};

enum GridMode {
    GRID_OFF,
    GRID_CIRCLE,
    GRID_SQUARE,
};

struct Settings {
    bool  sas_enabled;
    int   sas_mode;
    float sas_target_angle;

    int  volume;
    bool showPrediction;
    int  timeSpeed;

    float gravity;
    int   thrust;
    int   fuelRate;
    int   karmanLine;

    int sensorSlot[3];

    int grid_mode;
    int grid_step;

    bool grid_in_viewport;
    bool grid_in_minimap;
    bool grid_in_worldmap;
};

extern Settings settings;

void settings_init();
int  settings_timeSpeedValue();