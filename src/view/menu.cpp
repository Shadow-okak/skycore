#include "menu.h"
#include <Arduino.h>
#include <stdio.h>
#include <math.h>
#include "../pins/display.h"
#include "../settings.h"
#include "../sim/sensors.h"
#include "../sim/physics.h"
#include "../config.h"
#include "../language.h"

enum SettingType { ST_BOOL, ST_ENUM, ST_INT, ST_FLOAT, ST_PERCENT };

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

static const char* sasModeOpts[] = { L_SAS_PROGRADE, L_SAS_RETROGRADE, L_SAS_HOLD };

static const SettingDef sasItems[] = {
    { L_SAS_ENABLED, ST_BOOL, &settings.sas_enabled, 0, 0, 0, nullptr, 0 },
    { L_SAS_MODE,    ST_ENUM, &settings.sas_mode,    0, 2, 1, sasModeOpts, 3 },
};

static const char* timeSpeedOpts[] = { L_SPEED_1X, L_SPEED_2X, L_SPEED_3X, L_SPEED_5X };

static const SettingDef mainItems[] = {
    { L_SET_VOLUME,     ST_PERCENT, &settings.volume,         0, 100, 1, nullptr,        0 },
    { L_SET_PREDICTION, ST_BOOL,    &settings.showPrediction, 0, 0,   0, nullptr,        0 },
    { L_SET_TIMESPEED,  ST_ENUM,    &settings.timeSpeed,      0, 3,   1, timeSpeedOpts,  4 },
};

static const SettingDef physItems[] = {
    { L_SET_GRAVITY,  ST_FLOAT,   &settings.gravity,    0, 20,  0.1f, nullptr, 0 },
    { L_SET_THRUST,   ST_PERCENT, &settings.thrust,     0, 100, 1,    nullptr, 0 },
    { L_SET_FUELRATE, ST_PERCENT, &settings.fuelRate,   0, 100, 1,    nullptr, 0 },
    { L_SET_KARMAN,   ST_INT,     &settings.karmanLine, 0, 200, 1,    nullptr, 0 },
};

static const char* sensorOpts[SENSOR_COUNT];
static bool sensorOptsReady = false;

static SettingDef sensorItems[3] = {
    { L_SLOT_1, ST_ENUM, &settings.sensorSlot[0], 0, SENSOR_COUNT-1, 1, nullptr, SENSOR_COUNT },
    { L_SLOT_2, ST_ENUM, &settings.sensorSlot[1], 0, SENSOR_COUNT-1, 1, nullptr, SENSOR_COUNT },
    { L_SLOT_3, ST_ENUM, &settings.sensorSlot[2], 0, SENSOR_COUNT-1, 1, nullptr, SENSOR_COUNT },
};

static const char* gridTypeOpts[] = { L_GRID_OFF, L_GRID_CIRCLE, L_GRID_SQUARE };

static const SettingDef coordItems[] = {
    { L_SET_GRID_TYPE, ST_ENUM, &settings.grid_mode, 0, 2, 1, gridTypeOpts, 3 },
    { L_SET_GRID_STEP, ST_INT,  &settings.grid_step, 1, 500, 1, nullptr, 0 },
};

static const SettingDef graphItems[] = {
    { L_GRAPH_VIEWPORT, ST_BOOL, &settings.grid_in_viewport, 0, 0, 0, nullptr, 0 },
    { L_GRAPH_MINIMAP,  ST_BOOL, &settings.grid_in_minimap,  0, 0, 0, nullptr, 0 },
    { L_GRAPH_WORLDMAP, ST_BOOL, &settings.grid_in_worldmap, 0, 0, 0, nullptr, 0 },
};

static const TabDef TABS[] = {
    { L_TAB_SAS,     sasItems,   sizeof(sasItems)   / sizeof(SettingDef) },
    { L_TAB_MAIN,    mainItems,  sizeof(mainItems)  / sizeof(SettingDef) },
    { L_TAB_PHYSICS, physItems,  sizeof(physItems)  / sizeof(SettingDef) },
    { L_TAB_SENSORS, sensorItems, 3 },
    { L_TAB_COORD,   coordItems, sizeof(coordItems) / sizeof(SettingDef) },
    { L_TAB_GRAPH,   graphItems, sizeof(graphItems) / sizeof(SettingDef) },
};
constexpr int TAB_COUNT = sizeof(TABS) / sizeof(TabDef);

enum MenuState { MENU_TABS, MENU_ITEMS, MENU_EDIT_NUM };

static MenuState mState = MENU_TABS;
static int       mTab   = 0;
static int       mItem  = 0;
static int       scrollTab  = 0;
static int       scrollItem = 0;
static uint32_t  lastMove   = 0;

constexpr uint32_t MOVE_DELAY   = 180;
constexpr int      VISIBLE_TABS = 5;
constexpr int      VISIBLE_ITEMS = 4;

void menu_init() {
    if (!sensorOptsReady) {
        for (int i = 0; i < SENSOR_COUNT; i++) sensorOpts[i] = sensor_label(i);
        for (int i = 0; i < 3; i++) sensorItems[i].options = sensorOpts;
        sensorOptsReady = true;
    }
    mState = MENU_TABS;
    mTab = 0;
    mItem = 0;
    scrollTab = 0;
    scrollItem = 0;
}

static void clampScroll() {
    if (mTab < scrollTab) scrollTab = mTab;
    if (mTab >= scrollTab + VISIBLE_TABS) scrollTab = mTab - VISIBLE_TABS + 1;
    const TabDef& t = TABS[mTab];
    if (mItem < scrollItem) scrollItem = mItem;
    if (mItem >= scrollItem + VISIBLE_ITEMS) scrollItem = mItem - VISIBLE_ITEMS + 1;
    if (scrollItem < 0) scrollItem = 0;
    if (scrollTab < 0) scrollTab = 0;
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
            *p += dir;
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
            *p += dir;
            if (*p < 0)   *p = 0;
            if (*p > 100) *p = 100;
            break;
        }
    }
    if (s.ptr == &settings.sas_mode && settings.sas_mode == SAS_HOLD_ANGLE) {
        settings.sas_target_angle = ship.angle;
    }
}

void menu_update(uint32_t now, const Stick& in) {
    static bool lastBtn = false;
    if (!in.btn) lastBtn = false;
    bool btnEdge = (in.btn && !lastBtn);
    lastBtn = in.btn;

    if (btnEdge) {
        if (mState == MENU_ITEMS) {
            const TabDef& t = TABS[mTab];
            const SettingDef& s = t.settings[mItem];
            if (s.type == ST_BOOL) {
                changeValue(s, +1);
            } else {
                mState = MENU_EDIT_NUM;
            }
            lastMove = now;
            return;
        }
        if (mState == MENU_EDIT_NUM) {
            mState = MENU_ITEMS;
            lastMove = now;
            return;
        }
    }

    int ix = 0, iy = 0;
    if (fabs(in.x) > fabs(in.y)) {
        if (in.x >  0.6f) ix =  1;
        if (in.x < -0.6f) ix = -1;
    } else {
        if (in.y >  0.6f) iy =  1;
        if (in.y < -0.6f) iy = -1;
    }

    static int lastIx = 0;
    if (ix != 0 && lastIx == 0) {
        if (mState == MENU_TABS && ix > 0) {
            mState = MENU_ITEMS;
            lastIx = ix; lastMove = now; return;
        }
        if (mState == MENU_ITEMS && ix < 0) {
            mState = MENU_TABS;
            lastIx = ix; lastMove = now; return;
        }
    }

    bool hasDir = (ix != 0 || iy != 0);
    if (!hasDir) {
        lastMove = 0;
        lastIx = 0;
        return;
    }

    if (lastMove == 0) {
        lastMove = now;
    } else if (now - lastMove < MOVE_DELAY) {
        lastIx = ix;
        return;
    }

    switch (mState) {
        case MENU_TABS:
            if (iy != 0) {
                mTab = (mTab + iy + TAB_COUNT) % TAB_COUNT;
                mItem = 0;
                scrollItem = 0;
            }
            break;

        case MENU_ITEMS: {
            const TabDef& t = TABS[mTab];
            if (iy != 0) mItem = (mItem + iy + t.count) % t.count;
            break;
        }

        case MENU_EDIT_NUM: {
            const TabDef& t = TABS[mTab];
            const SettingDef& s = t.settings[mItem];
            if (iy != 0) changeValue(s, iy);
            break;
        }
    }

    clampScroll();
    lastMove = now;
    lastIx = ix;
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

    for (int i = scrollTab; i < TAB_COUNT && i < scrollTab + VISIBLE_TABS; i++) {
        int vy = i - scrollTab;
        int y = 12 + vy * 12 + 4;

        bool active   = (i == mTab);
        bool cursored = active && (mState == MENU_TABS);

        if (cursored) {
            g->drawBox(0, y - 9, SPLIT, 11);
            g->setDrawColor(0);
            g->drawUTF8(4, y, TABS[i].name);
            g->setDrawColor(1);
        } else if (active) {
            g->drawUTF8(4, y, TABS[i].name);
            g->drawFrame(0, y - 9, SPLIT, 11);
        } else {
            g->drawUTF8(4, y, TABS[i].name);
        }
    }

    if (scrollTab > 0) g->drawTriangle(SPLIT/2, 1, SPLIT/2-3, 5, SPLIT/2+3, 5);
    if (scrollTab + VISIBLE_TABS < TAB_COUNT)
        g->drawTriangle(SPLIT/2, 63, SPLIT/2-3, 59, SPLIT/2+3, 59);

    g->drawVLine(SPLIT, 0, 64);

    const TabDef& tab = TABS[mTab];
    for (int i = scrollItem; i < tab.count && i < scrollItem + VISIBLE_ITEMS; i++) {
        int vy = i - scrollItem;
        int y = 12 + vy * 13 + 4;

        const SettingDef& s = tab.settings[i];
        bool onRow = (mState != MENU_TABS) && (i == mItem);
        bool onVal = (mState == MENU_EDIT_NUM) && (i == mItem);

        char val[16];
        valueToString(s, val, sizeof(val));

        if (onRow && !onVal) {
            g->drawBox(SPLIT + 1, y - 9, 128 - SPLIT - 1, 11);
            g->setDrawColor(0);
        }
        g->drawUTF8(SPLIT + 3, y, s.name);

        int vw = g->getUTF8Width(val);
        int vx = 127 - vw - 2;
        if (onVal) {
            g->setDrawColor(1);
            g->drawBox(vx - 2, y - 9, vw + 4, 11);
            g->setDrawColor(0);
        }
        g->drawUTF8(vx, y, val);

        g->setDrawColor(1);
    }

    if (scrollItem > 0)
        g->drawTriangle(SPLIT + 4, 12, SPLIT + 1, 15, SPLIT + 7, 15);
    if (scrollItem + VISIBLE_ITEMS < tab.count)
        g->drawTriangle(SPLIT + 4, 62, SPLIT + 1, 59, SPLIT + 7, 59);
}