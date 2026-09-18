#include "menu.h"
#include <Arduino.h>
#include <stdio.h>
#include <math.h>
#include "../render/display.h"
#include "../settings.h"
#include "../config.h"
#include "../language.h"

enum SettingType {
    ST_BOOL,
    ST_ENUM,
    ST_INT,
    ST_FLOAT,
    ST_PERCENT,
};

struct SettingDef {
    const char* name;
    SettingType type;
    void*       ptr;
    float       min, max;
    float       step;
    const char** options;
    int         optionCount;
};

struct TabDef {
    const char*        name;
    const SettingDef*  settings;
    int                count;
};

static const SettingDef sasItems[] = {
    { L_SET_SAS_HOLD, ST_BOOL, &settings.sas_holdAngle, 0, 0, 0, nullptr, 0 },
};

static const char* timeSpeedOpts[] = { L_SPEED_1X, L_SPEED_2X, L_SPEED_3X, L_SPEED_5X };

static const SettingDef mainItems[] = {
    { L_SET_VOLUME,     ST_PERCENT, &settings.volume,         0, 100, 5, nullptr,        0 },
    { L_SET_TRAIL,      ST_BOOL,    &settings.showTrail,      0, 0,   0, nullptr,        0 },
    { L_SET_PREDICTION, ST_BOOL,    &settings.showPrediction, 0, 0,   0, nullptr,        0 },
    { L_SET_TIMESPEED,  ST_ENUM,    &settings.timeSpeed,      0, 3,   1, timeSpeedOpts,  4 },
};

static const SettingDef physItems[] = {
    { L_SET_GRAVITY,  ST_FLOAT,   &settings.gravity,    0, 20,  0.5f, nullptr, 0 },
    { L_SET_THRUST,   ST_PERCENT, &settings.thrust,     0, 100, 5,    nullptr, 0 },
    { L_SET_FUELRATE, ST_PERCENT, &settings.fuelRate,   0, 100, 5,    nullptr, 0 },
    { L_SET_KARMAN,   ST_INT,     &settings.karmanLine, 0, 200, 10,   nullptr, 0 },
};

static const TabDef TABS[] = {
    { L_TAB_SAS,     sasItems,  sizeof(sasItems)  / sizeof(SettingDef) },
    { L_TAB_MAIN,    mainItems, sizeof(mainItems) / sizeof(SettingDef) },
    { L_TAB_PHYSICS, physItems, sizeof(physItems) / sizeof(SettingDef) },
};
constexpr int TAB_COUNT = sizeof(TABS) / sizeof(TabDef);

enum MenuState { MENU_TABS, MENU_ITEMS, MENU_EDIT };

static MenuState mState = MENU_TABS;
static int       mTab   = 0;
static int       mItem  = 0;
static uint32_t  lastMove = 0;
constexpr uint32_t MOVE_DELAY = 200;

void menu_init() {
    mState = MENU_TABS;
    mTab   = 0;
    mItem  = 0;
}

static void changeValue(const SettingDef& s, int dir) {
    if (dir == 0) return;
    switch (s.type) {
        case ST_BOOL: {
            bool* p = (bool*)s.ptr;
            *p = !*p;
            break;
        }
        case ST_ENUM: {
            int* p = (int*)s.ptr;
            *p += dir;
            if (*p < 0) *p = s.optionCount - 1;
            if (*p >= s.optionCount) *p = 0;
            break;
        }
        case ST_INT: {
            int* p = (int*)s.ptr;
            *p += (int)(dir * s.step);
            if (*p < (int)s.min) *p = (int)s.min;
            if (*p > (int)s.max) *p = (int)s.max;
            break;
        }
        case ST_FLOAT: {
            float* p = (float*)s.ptr;
            *p += dir * s.step;
            if (*p < s.min) *p = s.min;
            if (*p > s.max) *p = s.max;
            break;
        }
        case ST_PERCENT: {
            int* p = (int*)s.ptr;
            *p += (int)(dir * s.step);
            if (*p < 0)   *p = 0;
            if (*p > 100) *p = 100;
            break;
        }
    }
}

void menu_update(uint32_t now, const Stick& in) {
    if (now - lastMove < MOVE_DELAY) return;

    int ix = 0, iy = 0;
    if (fabs(in.x) > fabs(in.y)) {
        if (in.x >  0.6f) ix =  1;
        if (in.x < -0.6f) ix = -1;
    } else {
        if (in.y >  0.6f) iy =  1;
        if (in.y < -0.6f) iy = -1;
    }

    if (ix == 0 && iy == 0) {
        lastMove = now - MOVE_DELAY + 60;
        return;
    }

    const TabDef& tab = TABS[mTab];

    switch (mState) {
        case MENU_TABS:
            if (iy != 0) {
                mTab = (mTab + iy + TAB_COUNT) % TAB_COUNT;
                mItem = 0;
            }
            if (ix > 0) mState = MENU_ITEMS;
            break;

        case MENU_ITEMS:
            if (iy != 0) {
                mItem = (mItem + iy + tab.count) % tab.count;
            }
            if (ix < 0) mState = MENU_TABS;
            if (ix > 0) {
                const SettingDef& s = tab.settings[mItem];
                if (s.type == ST_BOOL || s.type == ST_ENUM) {
                    changeValue(s, +1);
                } else {
                    mState = MENU_EDIT;
                }
            }
            break;

        case MENU_EDIT: {
            const SettingDef& s = tab.settings[mItem];
            if (ix < 0) {
                mState = MENU_ITEMS;
            } else if (iy != 0) {
                changeValue(s, iy);
            }
            break;
        }
    }

    lastMove = now;
}

static void valueToString(const SettingDef& s, char* buf, int bufSize) {
    switch (s.type) {
        case ST_BOOL: {
            bool v = *(bool*)s.ptr;
            snprintf(buf, bufSize, "%s", v ? L_VAL_ON : L_VAL_OFF);
            break;
        }
        case ST_ENUM: {
            int v = *(int*)s.ptr;
            if (v < 0 || v >= s.optionCount) v = 0;
            snprintf(buf, bufSize, "%s", s.options[v]);
            break;
        }
        case ST_INT: {
            int v = *(int*)s.ptr;
            snprintf(buf, bufSize, "%d", v);
            break;
        }
        case ST_FLOAT: {
            float v = *(float*)s.ptr;
            snprintf(buf, bufSize, "%.1f", v);
            break;
        }
        case ST_PERCENT: {
            int v = *(int*)s.ptr;
            snprintf(buf, bufSize, "%d%%", v);
            break;
        }
    }
}

void menu_draw() {
    U8G2* g = display_get();
    g->setFont(u8g2_font_6x12_t_cyrillic);
    g->setDrawColor(1);
    g->setMaxClipWindow();

    const int SPLIT = 44;

    for (int i = 0; i < TAB_COUNT; i++) {
        int y = 14 + i * 14;
        bool active = (i == mTab);
        bool cursored = active && (mState == MENU_TABS);

        if (cursored) {
            g->drawBox(0, y - 10, SPLIT, 12);
            g->setDrawColor(0);
            g->drawUTF8(4, y, TABS[i].name);
            g->setDrawColor(1);
        } else if (active) {
            g->drawUTF8(4, y, TABS[i].name);
            g->drawFrame(0, y - 10, SPLIT, 12);
        } else {
            g->drawUTF8(4, y, TABS[i].name);
        }
    }

    g->drawVLine(SPLIT, 0, 64);

    const TabDef& tab = TABS[mTab];
    for (int i = 0; i < tab.count; i++) {
        int y = 12 + i * 14;
        if (y > 62) break;

        const SettingDef& s = tab.settings[i];
        bool onRow = (mState != MENU_TABS) && (i == mItem);
        bool onVal = (mState == MENU_EDIT) && (i == mItem);

        char val[16];
        valueToString(s, val, sizeof(val));

        if (onRow && !onVal) {
            g->drawBox(SPLIT + 1, y - 10, 128 - SPLIT - 1, 12);
            g->setDrawColor(0);
        }
        g->drawUTF8(SPLIT + 3, y, s.name);

        int vw = g->getUTF8Width(val);
        int vx = 127 - vw - 2;
        if (onVal) {
            g->setDrawColor(1);
            g->drawBox(vx - 2, y - 10, vw + 4, 12);
            g->setDrawColor(0);
        }
        g->drawUTF8(vx, y, val);

        g->setDrawColor(1);
    }

    g->setDrawColor(1);
}