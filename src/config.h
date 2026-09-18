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

// ===== Экран =====
#define SCREEN_W      128
#define SCREEN_H      64

// ===== Общие константы =====
constexpr float DT         = 1.0f / 60.0f;
constexpr int   SPEED_MULT = 3;

// Кривая газа: S-curve для плавности
constexpr float THROTTLE_SMOOTH_K = 0.15f;  // 0..1, больше = быстрее отклик