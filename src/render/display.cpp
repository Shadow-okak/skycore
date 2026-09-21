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

void display_drawSpriteRotated(int x, int y, int w, int h,
                               const uint8_t* sprite, float angle) {
    float cx = w / 2.0f;
    float cy = h / 2.0f;
    float ca = cosf(angle);
    float sa = sinf(angle);

    int bytesPerRow = (w + 7) / 8;
    int maxR = (int)(sqrtf((float)(w*w + h*h)) / 2.0f) + 2;

    for (int dy = -maxR; dy <= maxR; dy++) {
        for (int dx = -maxR; dx <= maxR; dx++) {
            float sx = dx * ca + dy * sa + cx;
            float sy = -dx * sa + dy * ca + cy;

            int ix = (int)sx;
            int iy = (int)sy;
            if (ix < 0 || ix >= w || iy < 0 || iy >= h) continue;

            int byteIdx = iy * bytesPerRow + (ix >> 3);
            int bitIdx = 7 - (ix & 7);
            if (sprite[byteIdx] & (1 << bitIdx)) {
                u8g2.drawPixel(x + dx, y + dy);
            }
        }
    }
}