#pragma once
#include <U8g2lib.h>

U8G2* display_get();
void  display_init();
void  display_frameBegin();
void  display_frameEnd();

void  display_drawBar(int x, int y, int w, int h, float v01);
void  display_rotatePoint(float x, float y, float a,
                          int& outX, int& outY, int cx, int cy);