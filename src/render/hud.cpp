#include "hud.h"
#include "display.h"
#include "../core/physics.h"
#include "../config.h"

void hud_draw() {
    U8G2* g = display_get();
    g->setFont(u8g2_font_6x12_t_cyrillic);

    // Топливо сверху
    g->drawStr(66, 10, "Т");
    g->drawFrame(72, 2, 42, 8);
    int fill = (int)(40.0f * ship.fuel);
    if (fill > 0) g->drawBox(73, 3, fill, 6);

    // Снизу — вертикальная скорость и высота
    float vLocal = physics_localVerticalSpeed();
    float alt    = physics_altitude();

    g->setCursor(66, 62);
    if (vLocal > 0.5f)       g->print("^");
    else if (vLocal < -0.5f) g->print("v");
    else                     g->print(" ");
    g->print((int)fabs(vLocal));

    g->setCursor(104, 62);
    g->print((int)alt);
    g->print("м");
}