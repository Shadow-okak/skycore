#include "hud.h"
#include "display.h"
#include "../core/physics.h"
#include "../config.h"
#include "../language.h"

void hud_draw() {
    U8G2* g = display_get();
    g->setFont(u8g2_font_6x12_t_cyrillic);
    g->setDrawColor(1);

    g->drawUTF8(66, 10, L_HUD_FUEL);
    g->drawFrame(72, 2, 42, 8);
    int fill = (int)(40.0f * ship.fuel);
    if (fill > 0) g->drawBox(73, 3, fill, 6);

    float vLocal = physics_localVerticalSpeed();
    float alt    = physics_altitude();

    g->setCursor(66, 62);
    if (vLocal > 0.5f)       g->print(L_HUD_UP);
    else if (vLocal < -0.5f) g->print(L_HUD_DOWN);
    else                     g->print(" ");
    g->print((int)fabs(vLocal));

    g->setCursor(104, 62);
    g->print((int)alt);
    g->print(" ");
    g->drawUTF8(g->getCursorX(), 62, L_HUD_METERS);
}