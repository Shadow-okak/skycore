#pragma once

constexpr int TRAIL_MAX     = 200;
constexpr int PREDICT_MAX   = 250;
constexpr int PREDICT_STEPS = 2000;
constexpr int PREDICT_SKIP  = 8;

struct TrailPoint { float dx, dy; };

extern TrailPoint trail[TRAIL_MAX];
extern int trailHead, trailCount;

extern TrailPoint predict[PREDICT_MAX];
extern int predictCount;

void orbit_pushTrail();
void orbit_predict();
void orbit_clear();