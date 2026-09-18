#include "display.h"
#include <Wire.h>
#include <math.h>
#include "../config.h"

static U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

U8G2* display_get() { return &u8g2; }

void display_init() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    u8g2.begin();
    u8g2.setContrast(255);
    u8g2.setFont(u8g2_font_6x12_t_cyrillic);
    u8g2.setDrawColor(1);
}

void display_frameBegin() {
    u8g2.clearBuffer();
    u8g2.setDrawColor(1);
    u8g2.setMaxClipWindow();
}

void display_frameEnd() {
    u8g2.sendBuffer();
}

void display_drawBar(int x, int y, int w, int h, float v01) {
    if (v01 < 0) v01 = 0;
    if (v01 > 1) v01 = 1;
    u8g2.drawFrame(x, y, w, h);
    int fill = (int)((w - 2) * v01);
    if (fill > 0) u8g2.drawBox(x + 1, y + 1, fill, h - 2);
}

void display_rotatePoint(float x, float y, float a,
                         int& outX, int& outY, int cx, int cy) {
    float ca = cosf(a), sa = sinf(a);
    outX = cx + (int)(x * ca - y * sa);
    outY = cy + (int)(x * sa + y * ca);
}