#include "hud.h"
#include "display.h"
#include "../core/physics.h"
#include "../settings.h"
#include "../config.h"
#include "../language.h"

void hud_draw() {
    U8G2* g = display_get();
    g->setFont(u8g2_font_6x12_t_cyrillic);
    g->setDrawColor(1);

    g->drawFrame(102, 54, 24, 8);
    int fill = (int)(22.0f * ship.fuel);
    if (fill > 0) g->drawBox(103, 55, fill, 6);
}