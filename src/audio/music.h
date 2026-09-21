#pragma once
#include <stdint.h>
#include "../sim/physics.h"

void music_init();
void music_update(uint32_t now, const Ship& ship, bool onGround);