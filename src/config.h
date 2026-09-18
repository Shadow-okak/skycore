#pragma once

// ===== I2C (SSD1306) =====
#define PIN_I2C_SDA   5
#define PIN_I2C_SCL   4
#define OLED_ADDR     0x3C

// ===== Стик =====
#define PIN_STICK_X   6
#define PIN_STICK_Y   7
#define PIN_STICK_BTN 16

// ===== Зуммер =====
#define PIN_BUZZER    15

// ===== Переключатели режима =====
#define PIN_MENU_SW      17
#define PIN_WORLDMAP_SW  18

// ===== Экран =====
#define SCREEN_W      128
#define SCREEN_H      64

// ===== Общие константы =====
// TWO_PI уже определён в Arduino.h — свой не нужен
constexpr float DT         = 1.0f / 60.0f;
constexpr int   SPEED_MULT = 3;

// Кривая газа: плавность отклика
constexpr float THROTTLE_SMOOTH_K = 0.15f;