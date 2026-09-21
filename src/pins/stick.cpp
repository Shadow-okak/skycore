#include "stick.h"
#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "../config.h"
#include "../language.h"
#include "display.h"

int stick_centerX = 2048;
int stick_centerY = 2048;
int stick_rangeX  = 2048;
int stick_rangeY  = 2048;

constexpr float DEADZONE = 0.075f;

void stick_init() {
    pinMode(PIN_STICK_BTN, INPUT_PULLUP);
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
}

Stick stick_read() {
    int rawX = analogRead(PIN_STICK_X);
    int rawY = analogRead(PIN_STICK_Y);

    float nx = (float)(rawX - stick_centerX) / (float)stick_rangeX;
    float ny = (float)(rawY - stick_centerY) / (float)stick_rangeY;
    if (fabs(nx) < DEADZONE) nx = 0;
    if (fabs(ny) < DEADZONE) ny = 0;
    if (nx >  1.0f) nx =  1.0f;
    if (nx < -1.0f) nx = -1.0f;
    if (ny >  1.0f) ny =  1.0f;
    if (ny < -1.0f) ny = -1.0f;

    Stick s;
    s.x = ny;
    s.y = -nx;

    static bool lastBtn = false;
    static uint32_t lastChange = 0;
    bool raw = !digitalRead(PIN_STICK_BTN);
    uint32_t now = millis();
    if (raw != lastBtn && (now - lastChange) > 20) {
        lastBtn = raw;
        lastChange = now;
    }
    s.btn = lastBtn;
    return s;
}

// Один замер центра (усредняет за durationMs)
static void sample_center(int durationMs, int& outX, int& outY) {
    long sumX = 0, sumY = 0;
    int samples = 0;
    uint32_t t0 = millis();
    while (millis() - t0 < (uint32_t)durationMs) {
        sumX += analogRead(PIN_STICK_X);
        sumY += analogRead(PIN_STICK_Y);
        samples++;
        delay(2);
    }
    if (samples == 0) samples = 1;
    outX = sumX / samples;
    outY = sumY / samples;
}

void stick_calibrate() {
    U8G2* g = display_get();
    g->setFont(u8g2_font_6x12_t_cyrillic);
    g->setDrawColor(1);

    // ===== 1. Ждём 0.5 сек =====
    g->clearBuffer();
    g->drawUTF8(4, 20, L_CALIB_HEADER_1);
    g->drawUTF8(4, 38, L_CALIB_RELEASE);
    g->drawUTF8(4, 54, "0.5 сек");
    g->sendBuffer();
    delay(500);

    // ===== 2. Первый замер центра (0.5 сек) =====
    int cX1, cY1;
    sample_center(500, cX1, cY1);

    // ===== 3. Ждём 1 сек =====
    g->clearBuffer();
    g->drawUTF8(4, 20, L_CALIB_HEADER_1);
    g->drawUTF8(4, 38, L_CALIB_RELEASE);
    g->drawUTF8(4, 54, "1 сек");
    g->sendBuffer();
    delay(1000);

    // ===== 4. Второй замер центра (0.5 сек) =====
    int cX2, cY2;
    sample_center(500, cX2, cY2);

    // Усредняем два замера
    stick_centerX = (cX1 + cX2) / 2;
    stick_centerY = (cY1 + cY2) / 2;

    // ===== 5. Пауза 0.5 сек перед второй фазой =====
    g->clearBuffer();
    g->drawUTF8(4, 22, L_CALIB_HEADER_2);
    g->drawUTF8(4, 44, "Приготовься");
    g->sendBuffer();
    delay(500);

    // Ждём отпускания кнопки (на случай, если её держали)
    while (!digitalRead(PIN_STICK_BTN)) delay(10);
    delay(150);

    // ===== 6. Фаза 2: размах от пользователя =====
    int minX = stick_centerX, maxX = stick_centerX;
    int minY = stick_centerY, maxY = stick_centerY;
    bool done = false;

    while (!done) {
        int rx = analogRead(PIN_STICK_X);
        int ry = analogRead(PIN_STICK_Y);
        if (rx < minX) minX = rx;
        if (rx > maxX) maxX = rx;
        if (ry < minY) minY = ry;
        if (ry > maxY) maxY = ry;

        g->clearBuffer();
        g->drawUTF8(4, 12, L_CALIB_HEADER_2);
        g->drawUTF8(4, 26, L_CALIB_MOVE);
        g->drawUTF8(4, 40, L_CALIB_DONE_SW);
        g->setCursor(4, 58);
        g->print("X:");
        g->print(minX);
        g->print("-");
        g->print(maxX);
        g->sendBuffer();

        if (!digitalRead(PIN_STICK_BTN)) {
            delay(30);
            if (!digitalRead(PIN_STICK_BTN)) done = true;
        }
        delay(30);
    }

    int dxNeg = stick_centerX - minX, dxPos = maxX - stick_centerX;
    int dyNeg = stick_centerY - minY, dyPos = maxY - stick_centerY;
    stick_rangeX = (dxNeg > dxPos) ? dxNeg : dxPos;
    stick_rangeY = (dyNeg > dyPos) ? dyNeg : dyPos;
    if (stick_rangeX < 100) stick_rangeX = 2048;
    if (stick_rangeY < 100) stick_rangeY = 2048;

    // ===== Показываем результаты =====
    g->clearBuffer();
    g->setCursor(0, 14);
    g->print(L_CALIB_CENTER);
    g->print(stick_centerX);
    g->print(" ");
    g->print(stick_centerY);
    g->setCursor(0, 30);
    g->print(L_CALIB_RANGE);
    g->print(stick_rangeX);
    g->print(" ");
    g->print(stick_rangeY);
    g->drawUTF8(0, 50, L_CALIB_DONE);
    g->sendBuffer();
    delay(700);
}