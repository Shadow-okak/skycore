#include "mode.h"
#include <Arduino.h>
#include "../config.h"

void mode_init() {
    pinMode(PIN_MENU_SW,     INPUT_PULLUP);
    pinMode(PIN_WORLDMAP_SW, INPUT_PULLUP);
}

ScreenMode mode_read() {
    bool menu     = !digitalRead(PIN_MENU_SW);
    bool worldmap = !digitalRead(PIN_WORLDMAP_SW);
    if (menu)     return MODE_MENU;
    if (worldmap) return MODE_WORLDMAP;
    return MODE_FLIGHT;
}