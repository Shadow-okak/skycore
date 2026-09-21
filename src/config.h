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
constexpr float DT         = 1.0f / 60.0f;
constexpr int   SPEED_MULT = 3;
constexpr float THROTTLE_SMOOTH_K = 0.15f;

// ===== Вращение (инерция) =====
constexpr float ROT_ACCEL      = 8.0f;
constexpr float ROT_DAMP_SPACE = 1.5f;
constexpr float ROT_DAMP_ATMO  = 4.0f;
constexpr float SAS_KP         = 6.0f;
constexpr float SAS_KD         = 3.0f;

// ===== Масса =====
constexpr float MASS_DRY       = 300.0f;
constexpr float MASS_FUEL_MAX  = 700.0f;
constexpr float THRUST_FORCE   = 12000.0f;

// ===== Аэродинамика =====
constexpr float DRAG_K         = 0.008f;

// ===== Зуммер (LEDC) =====
#define BUZZER_LEDC_CHANNEL  0
#define BUZZER_LEDC_RES      8
constexpr int BUZZER_MAX_DUTY = 127;