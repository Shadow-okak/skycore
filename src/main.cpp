#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "config.h"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// ===== Ускорение времени =====
constexpr int SPEED_MULT = 3;

// ===== Калибровка =====
int centerX = 2048, centerY = 2048;
int rangeX  = 2048, rangeY  = 2048;
constexpr float DEADZONE = 0.075f;

// ===== Мир =====
struct World {
    float planetX, planetY;
    float planetR;
    float gravity;
    float karmanLow;
    float karmanHigh;
};

struct Ship {
    float x, y;
    float vx, vy;
    float angle;
    float fuel;
    float throttle;
};

constexpr float SHIP_R = 5.0f;
constexpr float DT = 1.0f / 60.0f;

World world;
Ship ship;

// ===== Трейл прошлого =====
constexpr int TRAIL_MAX = 200;
struct TrailPoint { float dx, dy; };
TrailPoint trail[TRAIL_MAX];
int trailHead = 0;
int trailCount = 0;

void pushTrail() {
    trail[trailHead].dx = ship.x - world.planetX;
    trail[trailHead].dy = ship.y - world.planetY;
    trailHead = (trailHead + 1) % TRAIL_MAX;
    if (trailCount < TRAIL_MAX) trailCount++;
}

void clearTrail() {
    trailHead = 0;
    trailCount = 0;
}

// ===== Предсказание орбиты (баллистическое, без тяги) =====
constexpr int PREDICT_STEPS = 2000;    // 2000 шагов × 1/60 ≈ 33 сек вперёд
constexpr int PREDICT_SKIP  = 8;       // каждая 8-я точка
constexpr int PREDICT_MAX   = 250;

TrailPoint predict[PREDICT_MAX];
int predictCount = 0;

void predictOrbit() {
    float px = ship.x, py = ship.y;
    float pvx = ship.vx, pvy = ship.vy;
    predictCount = 0;

    for (int i = 0; i < PREDICT_STEPS && predictCount < PREDICT_MAX; i++) {
        float dx = world.planetX - px;
        float dy = world.planetY - py;
        float d = sqrtf(dx*dx + dy*dy);
        if (d > 1.0f) {
            float g = world.gravity * (world.planetR * world.planetR) / (d * d);
            pvx += (dx / d) * g * DT;
            pvy += (dy / d) * g * DT;
        }
        px += pvx * DT;
        py += pvy * DT;

        if (i % PREDICT_SKIP == 0) {
            predict[predictCount].dx = px - world.planetX;
            predict[predictCount].dy = py - world.planetY;
            predictCount++;
        }

        // Если врезались — прекращаем
        float ddx = px - world.planetX;
        float ddy = py - world.planetY;
        if (ddx*ddx + ddy*ddy < world.planetR * world.planetR) break;
    }
}

// ===== Стик =====
struct Stick { float x, y; bool btn; };

Stick readStick() {
    int rawX = analogRead(PIN_STICK_X);
    int rawY = analogRead(PIN_STICK_Y);

    float nx = (float)(rawX - centerX) / (float)rangeX;
    float ny = (float)(rawY - centerY) / (float)rangeY;
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

// ===== Калибровка =====
void calibrate() {
    u8g2.setFont(u8g2_font_6x12_t_cyrillic);

    u8g2.clearBuffer();
    u8g2.drawStr(4, 20, "Калибровка 1/2");
    u8g2.drawStr(4, 38, "ОТПУСТИ стик");
    u8g2.drawStr(4, 54, "0.5 сек");
    u8g2.sendBuffer();

    long sumX = 0, sumY = 0;
    int samples = 0;
    uint32_t t0 = millis();
    while (millis() - t0 < 500) {
        sumX += analogRead(PIN_STICK_X);
        sumY += analogRead(PIN_STICK_Y);
        samples++;
        delay(2);
    }
    centerX = sumX / samples;
    centerY = sumY / samples;

    while (!digitalRead(PIN_STICK_BTN)) delay(10);
    delay(150);

    int minX = centerX, maxX = centerX;
    int minY = centerY, maxY = centerY;
    bool done = false;

    while (!done) {
        int rx = analogRead(PIN_STICK_X);
        int ry = analogRead(PIN_STICK_Y);
        if (rx < minX) minX = rx;
        if (rx > maxX) maxX = rx;
        if (ry < minY) minY = ry;
        if (ry > maxY) maxY = ry;

        u8g2.clearBuffer();
        u8g2.drawStr(4, 12, "Калибровка 2/2");
        u8g2.drawStr(4, 26, "Води стиком");
        u8g2.drawStr(4, 40, "SW = готово");
        u8g2.setCursor(4, 58);
        u8g2.print("X:");
        u8g2.print(minX);
        u8g2.print("-");
        u8g2.print(maxX);
        u8g2.sendBuffer();

        if (!digitalRead(PIN_STICK_BTN)) {
            delay(30);
            if (!digitalRead(PIN_STICK_BTN)) done = true;
        }
        delay(30);
    }

    int dxNeg = centerX - minX, dxPos = maxX - centerX;
    int dyNeg = centerY - minY, dyPos = maxY - centerY;
    rangeX = (dxNeg > dxPos) ? dxNeg : dxPos;
    rangeY = (dyNeg > dyPos) ? dyNeg : dyPos;
    if (rangeX < 100) rangeX = 2048;
    if (rangeY < 100) rangeY = 2048;

    u8g2.clearBuffer();
    u8g2.setCursor(0, 14);
    u8g2.print("Ц:");
    u8g2.print(centerX);
    u8g2.print(" ");
    u8g2.print(centerY);
    u8g2.setCursor(0, 30);
    u8g2.print("Р:");
    u8g2.print(rangeX);
    u8g2.print(" ");
    u8g2.print(rangeY);
    u8g2.drawStr(0, 50, "Готово!");
    u8g2.sendBuffer();
    delay(1000);
}

// ===== Инициализация =====
void initGame() {
    world.planetX    = 64;
    world.planetY    = 100;
    world.planetR    = 80;
    world.gravity    = 8.0f;
    world.karmanLow  = 30.0f;
    world.karmanHigh = 90.0f;

    ship.x = 64;
    ship.y = 0;
    ship.vx = 0;
    ship.vy = 0;
    ship.angle = 0;
    ship.fuel = 1.0f;
    ship.throttle = 0;

    clearTrail();
}

// ===== Физика =====
constexpr float ROT_RATE = 2.5f;
uint32_t lastPhys = 0;

void physics(const Stick& in) {
    ship.angle += in.x * ROT_RATE * DT;
    // Нормализация: 0..2π
    if (ship.angle >= TWO_PI) ship.angle -= TWO_PI;
    if (ship.angle < 0)       ship.angle += TWO_PI;

    float throttle = in.y;
    if (throttle < 0.05f) throttle = 0;
    if (throttle > 1.0f)  throttle = 1.0f;

    if (throttle > 0 && ship.fuel > 0) {
        ship.throttle = throttle;
        float noseX =  sinf(ship.angle);
        float noseY = -cosf(ship.angle);
        float power = throttle * 100.0f;
        ship.vx += noseX * power * DT;
        ship.vy += noseY * power * DT;
        ship.fuel -= throttle * 0.15f * DT;
        if (ship.fuel < 0) ship.fuel = 0;
    } else {
        ship.throttle = 0;
    }

    float dx = world.planetX - ship.x;
    float dy = world.planetY - ship.y;
    float d  = sqrtf(dx*dx + dy*dy);
    if (d > 1.0f) {
        float g = world.gravity * (world.planetR * world.planetR) / (d * d);
        ship.vx += (dx / d) * g * DT;
        ship.vy += (dy / d) * g * DT;
    }

    ship.x += ship.vx * DT;
    ship.y += ship.vy * DT;

    float px = ship.x - world.planetX;
    float py = ship.y - world.planetY;
    float pd = sqrtf(px*px + py*py);
    float collisionR = world.planetR + SHIP_R;
    if (pd < collisionR) {
        if (pd < 0.01f) pd = 0.01f;
        float nx2 = px / pd;
        float ny2 = py / pd;
        ship.x = world.planetX + nx2 * collisionR;
        ship.y = world.planetY + ny2 * collisionR;
        float vn = ship.vx * nx2 + ship.vy * ny2;
        if (vn < 0) {
            ship.vx -= vn * nx2;
            ship.vy -= vn * ny2;
        }
    }
}

// ===== Поворот точки =====
void rotatePoint(float x, float y, float a, int& outX, int& outY, int cx, int cy) {
    float ca = cosf(a), sa = sinf(a);
    outX = cx + (int)(x * ca - y * sa);
    outY = cy + (int)(x * sa + y * ca);
}

// ===== Камера =====
float camRot = 0;

void computeCamera() {
    float toPlanetX = world.planetX - ship.x;
    float toPlanetY = world.planetY - ship.y;
    float d = sqrtf(toPlanetX*toPlanetX + toPlanetY*toPlanetY);
    if (d < 0.01f) { toPlanetX = 0; toPlanetY = 1; d = 1; }
    toPlanetX /= d; toPlanetY /= d;

    float alt = d - world.planetR;
    float t = 0;
    if (alt < world.karmanLow) t = 1;
    else if (alt < world.karmanHigh)
        t = (world.karmanHigh - alt) / (world.karmanHigh - world.karmanLow);

    float lx = -toPlanetX, ly = -toPlanetY;
    float wx = 0, wy = -1;
    float cux = lx * t + wx * (1 - t);
    float cuy = ly * t + wy * (1 - t);
    float cl = sqrtf(cux*cux + cuy*cuy);
    if (cl > 0.01f) { cux /= cl; cuy /= cl; }

    camRot = atan2f(-cux, -cuy);
}

// ===== ЛЕВАЯ ПОЛОВИНА =====
void drawViewport() {
    u8g2.setClipWindow(0, 0, 62, 63);

    const int cx = 31, cy = 32;

    float dx = world.planetX - ship.x;
    float dy = world.planetY - ship.y;
    float c = cosf(camRot), s = sinf(camRot);
    float rx = dx * c - dy * s;
    float ry = dx * s + dy * c;
    int px = cx + (int)rx;
    int py = cy + (int)ry;
    int pr = (int)world.planetR;

    int top = py - pr, bot = py + pr;
    if (top < 0)  top = 0;
    if (bot > 63) bot = 63;
    for (int y = top; y <= bot; y++) {
        int dyy = y - py;
        int dx2 = pr*pr - dyy*dyy;
        if (dx2 < 0) continue;
        int dxx = (int)sqrtf((float)dx2);
        int x1 = px - dxx, x2 = px + dxx;
        if (x2 < 0 || x1 > 62) continue;
        if (x1 < 0)  x1 = 0;
        if (x2 > 62) x2 = 62;
        u8g2.drawHLine(x1, y, x2 - x1 + 1);
    }

    float screenAngle = ship.angle + camRot;
    int nx, ny, lbx, lby, rbx, rby;
    rotatePoint(0, -8, screenAngle, nx, ny, cx, cy);
    rotatePoint(-6, 6, screenAngle, lbx, lby, cx, cy);
    rotatePoint(6, 6, screenAngle, rbx, rby, cx, cy);

    u8g2.drawTriangle(nx, ny, lbx, lby, rbx, rby);
    u8g2.drawLine(lbx, lby, rbx, rby);

    if (ship.throttle > 0.05f) {
        int tx, ty;
        rotatePoint(0, 6, screenAngle, tx, ty, cx, cy);
        float sTailX = -sinf(screenAngle);
        float sTailY =  cosf(screenAngle);
        int bx = tx + (int)(sTailX * 5);
        int by = ty + (int)(sTailY * 5);
        u8g2.drawLine(tx, ty, bx, by);
    }

    u8g2.setMaxClipWindow();
}

// ===== ПРАВАЯ ПОЛОВИНА =====
void drawMinimap() {
    u8g2.setClipWindow(64, 0, 127, 63);

    const int cx = 96;
    const int cy = 34;
    const float S = 0.10f;

    int pr = (int)(world.planetR * S);
    if (pr < 4) pr = 4;
    u8g2.drawDisc(cx, cy, pr);

    int kr = (int)((world.planetR + world.karmanLow) * S);
    for (int a = 0; a < 360; a += 12) {
        float rad = a * 3.14159f / 180.0f;
        int x = cx + (int)(cosf(rad) * kr);
        int y = cy + (int)(sinf(rad) * kr);
        if (x < 65 || x > 126) continue;
        if (y < 2 || y > 62) continue;
        u8g2.drawPixel(x, y);
        u8g2.drawPixel(x + 1, y);
    }

    // ===== ПРЕДСКАЗАНИЕ ОРБИТЫ (рисуем тонкой линией точек) =====
    for (int i = 0; i < predictCount; i++) {
        int sx = cx + (int)(predict[i].dx * S);
        int sy = cy + (int)(predict[i].dy * S);
        if (sx < 65 || sx > 126) continue;
        if (sy < 2 || sy > 62) continue;
        u8g2.drawPixel(sx, sy);
    }

    // ===== ТРЕЙЛ ПРОШЛОГО =====
    for (int i = 0; i < trailCount; i++) {
        int idx = (trailHead - trailCount + i + TRAIL_MAX) % TRAIL_MAX;
        int sx = cx + (int)(trail[idx].dx * S);
        int sy = cy + (int)(trail[idx].dy * S);
        if (sx < 65 || sx > 126) continue;
        if (sy < 2 || sy > 62) continue;
        // Прошлое — только каждая 2-я точка, чтобы отличать от будущего
        if (i % 2 == 0) u8g2.drawPixel(sx, sy);
    }

    // ===== КОРАБЛЬ =====
    float dx = ship.x - world.planetX;
    float dy = ship.y - world.planetY;
    int sx = cx + (int)(dx * S);
    int sy = cy + (int)(dy * S);

    if (sx < 66)  sx = 66;
    if (sx > 125) sx = 125;
    if (sy < 14)  sy = 14;
    if (sy > 52)  sy = 52;

    u8g2.drawDisc(sx, sy, 2);

    int noseX = sx + (int)(sinf(ship.angle) * 5);
    int noseY = sy - (int)(cosf(ship.angle) * 5);
    u8g2.drawLine(sx, sy, noseX, noseY);

    u8g2.setMaxClipWindow();
}

// ===== HUD =====
void drawHud() {
    u8g2.setFont(u8g2_font_6x12_t_cyrillic);

    u8g2.drawStr(66, 10, "Т");
    u8g2.drawFrame(72, 2, 42, 8);
    int fill = (int)(40.0f * ship.fuel);
    if (fill > 0) u8g2.drawBox(73, 3, fill, 6);

    float dx = ship.x - world.planetX;
    float dy = ship.y - world.planetY;
    float dist = sqrtf(dx*dx + dy*dy);
    float alt = dist - world.planetR;
    if (alt < 0) alt = 0;

    float toPlanetX = world.planetX - ship.x;
    float toPlanetY = world.planetY - ship.y;
    float dl = sqrtf(toPlanetX*toPlanetX + toPlanetY*toPlanetY);
    float vLocal = 0;
    if (dl > 0.01f) {
        vLocal = -(ship.vx * toPlanetX + ship.vy * toPlanetY) / dl;
    }

    u8g2.setCursor(66, 62);
    if (vLocal > 0.5f)       u8g2.print("^");
    else if (vLocal < -0.5f) u8g2.print("v");
    else                     u8g2.print(" ");
    u8g2.print((int)fabs(vLocal));

    u8g2.setCursor(104, 62);
    u8g2.print((int)alt);
    u8g2.print("м");
}

void render() {
    u8g2.clearBuffer();

    computeCamera();
    drawViewport();
    drawMinimap();

    u8g2.drawVLine(63, 0, 64);

    drawHud();

    u8g2.sendBuffer();
}

// ===== Serial: события и статус =====
bool wasOnGround   = false;
bool wasFuelEmpty  = false;
bool wasThrusting  = false;

void logEvents() {
    float dx = ship.x - world.planetX;
    float dy = ship.y - world.planetY;
    float dist = sqrtf(dx*dx + dy*dy);
    float alt = dist - world.planetR;
    float collisionR = world.planetR + SHIP_R;
    bool onGround = (dist <= collisionR + 0.5f);

    // Посадка
    if (onGround && !wasOnGround) {
        float v = sqrtf(ship.vx*ship.vx + ship.vy*ship.vy);
        Serial.printf("[EV] Посадка  v=%.1f м/с\n", v);
    }
    // Взлёт
    if (!onGround && wasOnGround) {
        Serial.println("[EV] Взлёт");
    }
    wasOnGround = onGround;

    // Топливо кончилось
    bool fuelEmpty = (ship.fuel <= 0);
    if (fuelEmpty && !wasFuelEmpty) {
        Serial.println("[EV] Топливо кончилось");
    }
    wasFuelEmpty = fuelEmpty;

    // Начало/конец тяги
    bool thrusting = ship.throttle > 0.05f;
    if (thrusting && !wasThrusting) {
        Serial.printf("[EV] Тяга включена  %.0f%%\n", ship.throttle * 100);
    }
    if (!thrusting && wasThrusting) {
        Serial.println("[EV] Тяга выключена");
    }
    wasThrusting = thrusting;
}

void logStatus() {
    float dx = ship.x - world.planetX;
    float dy = ship.y - world.planetY;
    float dist = sqrtf(dx*dx + dy*dy);
    float alt = dist - world.planetR;
    if (alt < 0) alt = 0;

    float speed = sqrtf(ship.vx*ship.vx + ship.vy*ship.vy);
    float toPlanetX = world.planetX - ship.x;
    float toPlanetY = world.planetY - ship.y;
    float dl = sqrtf(toPlanetX*toPlanetX + toPlanetY*toPlanetY);
    float vLocal = 0;
    if (dl > 0.01f) vLocal = -(ship.vx * toPlanetX + ship.vy * toPlanetY) / dl;

    Serial.printf("[ST] alt=%.0fм  v=%.1f  vл=%.1f  угол=%.0f°  топ=%d%%\n",
                  alt, speed, vLocal,
                  ship.angle * 57.2958f,
                  (int)(ship.fuel * 100));
}

// ===== Setup / Loop =====
void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.println("\n=== SkyCore ===");
    Serial.printf("Ускорение времени: x%d\n", SPEED_MULT);

    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    u8g2.begin();
    u8g2.setContrast(255);

    pinMode(PIN_STICK_BTN, INPUT_PULLUP);
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    u8g2.setFont(u8g2_font_6x12_t_cyrillic);

    calibrate();

    Serial.printf("Калибровка: cx=%d cy=%d rx=%d ry=%d\n",
                  centerX, centerY, rangeX, rangeY);

    initGame();
    Serial.println("Игра запущена. Жду событий...");
}

void loop() {
    uint32_t now = millis();

    if (now - lastPhys >= (uint32_t)(DT * 1000)) {
        lastPhys = now;
        Stick in = readStick();
        for (int i = 0; i < SPEED_MULT; i++) {
            physics(in);
        }

        static uint32_t lastTrail = 0;
        if (now - lastTrail >= 50) {
            lastTrail = now;
            pushTrail();
        }

        // Предсказание орбиты — 5 раз в секунду (каждые 200 мс)
        static uint32_t lastPredict = 0;
        if (now - lastPredict >= 200) {
            lastPredict = now;
            predictOrbit();
        }

        // Проверка событий — каждый физический шаг
        logEvents();
    }

    // Статус — раз в секунду, без спама
    static uint32_t lastStatus = 0;
    if (now - lastStatus >= 1000) {
        lastStatus = now;
        logStatus();
    }

    static uint32_t lastRender = 0;
    if (now - lastRender >= 33) {
        lastRender = now;
        render();
    }
}