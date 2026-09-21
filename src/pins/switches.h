#pragma once

enum ScreenMode { MODE_FLIGHT, MODE_MENU, MODE_WORLDMAP };

void       switches_init();
ScreenMode switches_read();