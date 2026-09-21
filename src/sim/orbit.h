#pragma once
#include <stdint.h>

constexpr int PREDICT_MAX   = 800;
constexpr int PREDICT_STEPS = 8000;
constexpr int PREDICT_SKIP  = 10;

struct TrailPoint { float x, y; };

extern TrailPoint predict[PREDICT_MAX];
extern int predictCount;

void orbit_predict();
void orbit_clear();