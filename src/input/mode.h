#pragma once

enum ScreenMode { MODE_FLIGHT, MODE_MENU, MODE_WORLDMAP };

void       mode_init();
ScreenMode mode_read();