#pragma once
#include <stdint.h>
#include "../input/stick.h"

void menu_init();
void menu_update(uint32_t now, const Stick& in);
void menu_draw();