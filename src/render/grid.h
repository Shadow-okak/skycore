#pragma once
#include <U8G2lib.h>

void grid_draw_circle(U8G2* g, int cx, int cy, int step_px,
                      int x0, int y0, int x1, int y1);
void grid_draw_square(U8G2* g, int cx, int cy, int step_px,
                      int x0, int y0, int x1, int y1);