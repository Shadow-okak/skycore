#pragma once
#include <stdint.h>
#include "core/physics.h"

void music_init();
void music_update(uint32_t now, const Ship& ship, bool onGround);
void music_playStartup();