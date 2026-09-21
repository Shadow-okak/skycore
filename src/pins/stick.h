#pragma once

struct Stick { float x, y; bool btn; };

void  stick_init();
void  stick_calibrate();
Stick stick_read();

extern int stick_centerX, stick_centerY;
extern int stick_rangeX,  stick_rangeY;