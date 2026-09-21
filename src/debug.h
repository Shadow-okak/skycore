#pragma once
#include <stdint.h>
#include "sim/physics.h"

void debug_init();
void debug_tick(uint32_t now, const Ship& ship);