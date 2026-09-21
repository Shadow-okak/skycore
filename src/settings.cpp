#include "settings.h"
#include "core/sensors.h"

Settings settings;

void settings_init() {
    settings.sas_enabled      = false;
    settings.sas_mode         = SAS_PROGRADE;
    settings.sas_target_angle = 0.0f;

    settings.volume          = 40;
    settings.showPrediction  = true;
    settings.timeSpeed       = 2;

    settings.gravity         = 8.0f;
    settings.thrust          = 50;
    settings.fuelRate        = 15;
    settings.karmanLine      = 30;

    settings.sensorSlot[0] = SENSOR_SPEED;
    settings.sensorSlot[1] = SENSOR_ALTITUDE;
    settings.sensorSlot[2] = SENSOR_FUEL;

    settings.grid_mode           = GRID_OFF;
    settings.grid_step           = 50;
    settings.grid_in_viewport    = false;
    settings.grid_in_minimap     = true;
    settings.grid_in_worldmap    = true;
}

int settings_timeSpeedValue() {
    static const int v[] = {1, 2, 3, 5};
    int i = settings.timeSpeed;
    if (i < 0) i = 0;
    if (i > 3) i = 3;
    return v[i];
}