#include <Arduino.h>
#include <lvgl.h>
#include <SPI.h>
#include <Wire.h>
#include "display_driver.h"
#include "touch_driver.h"
#include "llm_handler.h"

// Audio (will be implemented with ESP32-audioI2S)
#define I2S_BCLK   15
#define I2S_LRCK   16
#define I2S_DOUT   17
#define MIC_SCK    41
#define MIC_WS     42
#define MIC_DATA   2

// その他
#define TF_CS      39
#define BATTERY    6

// ===== ディスプレイ設定 =====
#define SCREEN_WIDTH  360
#define SCREEN_HEIGHT 360
#define CIRCLE_RADIUS 180

// ===== LVGL用バッファ =====
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[SCREEN_WIDTH * 60];
static lv_color_t buf2[SCREEN_WIDTH * 60];
static lv_disp_drv_t disp_drv;

// ===== ハードウェアドライバ =====
DisplayDriver* display;
TouchDriver* touch;
LLMHandler* llm;

// ===== カービィキャラクター =====
lv_obj_t *face_circle;
lv_obj_t *left_eye;
lv_obj_t *right_eye;
lv_obj_t *mouth;

// アニメーション状態
typedef enum {
    ANIM_IDLE,
    ANIM_BLINK,
    ANIM_HAPPY,
    ANIM_SURPRISE,
    ANIM_TALK
} AnimState;

AnimState current_anim = ANIM_IDLE;
uint32_t last_anim_time = 0;
uint32_t blink_timer = 0;

// ===== LVGL入力デバイスコールバック =====
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    if (touch->read() && touch->isTouched()) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = touch->getX();
        data->point.y = touch->getY();
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

// ===== LVGLの初期化 =====
void lvgl_init() {
    Serial.println("LVGL初期化中...");
    
    lv_init();
    
    // ディスプレイバッファ設定（ダブルバッファ）
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, SCREEN_WIDTH * 60);
    
    // ディスプレイドライバ登録
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = DisplayDriver::lvgl_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
    
    // タッチ入力ドライバ登録
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);
    
    Serial.println("LVGL初期化完了!");
}

// ===== カービィ風キャラクターの作成 =====
void create_kirby_character() {
    // 背景をピンク系のグラデーションに
    lv_obj_t *bg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(bg, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(bg, lv_color_hex(0xFFE8F5), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(bg, lv_color_hex(0xFFB3E5), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(bg, LV_GRAD_DIR_VER, LV_PART_MAIN);
    
    // 丸い顔 (ピンク色)
    face_circle = lv_obj_create(lv_scr_act());
    lv_obj_set_size(face_circle, 280, 280);
    lv_obj_align(face_circle, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(face_circle, lv_color_hex(0xFFB4D5), LV_PART_MAIN);
    lv_obj_set_style_border_width(face_circle, 4, LV_PART_MAIN);
    lv_obj_set_style_border_color(face_circle, lv_color_hex(0xFF8AC7), LV_PART_MAIN);
    lv_obj_set_style_radius(face_circle, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    
    // 左目 (楕円形)
    left_eye = lv_obj_create(face_circle);
    lv_obj_set_size(left_eye, 45, 65);
    lv_obj_align(left_eye, LV_ALIGN_CENTER, -45, -20);
    lv_obj_set_style_bg_color(left_eye, lv_color_hex(0x1A1A4D), LV_PART_MAIN);
    lv_obj_set_style_border_width(left_eye, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(left_eye, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    
    // 白いハイライト
    lv_obj_t *left_highlight = lv_obj_create(left_eye);
    lv_obj_set_size(left_highlight, 15, 20);
    lv_obj_align(left_highlight, LV_ALIGN_TOP_LEFT, 8, 8);
    lv_obj_set_style_bg_color(left_highlight, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_radius(left_highlight, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_border_width(left_highlight, 0, LV_PART_MAIN);
    
    // 右目 (楕円形)
    right_eye = lv_obj_create(face_circle);
    lv_obj_set_size(right_eye, 45, 65);
    lv_obj_align(right_eye, LV_ALIGN_CENTER, 45, -20);
    lv_obj_set_style_bg_color(right_eye, lv_color_hex(0x1A1A4D), LV_PART_MAIN);
    lv_obj_set_style_border_width(right_eye, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(right_eye, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    
    // 白いハイライト
    lv_obj_t *right_highlight = lv_obj_create(right_eye);
    lv_obj_set_size(right_highlight, 15, 20);
    lv_obj_align(right_highlight, LV_ALIGN_TOP_LEFT, 8, 8);
    lv_obj_set_style_bg_color(right_highlight, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_radius(right_highlight, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_border_width(right_highlight, 0, LV_PART_MAIN);
    
    // 口 (小さな楕円)
    mouth = lv_obj_create(face_circle);
    lv_obj_set_size(mouth, 30, 15);
    lv_obj_align(mouth, LV_ALIGN_CENTER, 0, 40);
    lv_obj_set_style_bg_color(mouth, lv_color_hex(0xD84A6F), LV_PART_MAIN);
    lv_obj_set_style_border_width(mouth, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(mouth, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    
    // 頬の赤み (左)
    lv_obj_t *left_cheek = lv_obj_create(face_circle);
    lv_obj_set_size(left_cheek, 50, 40);
    lv_obj_align(left_cheek, LV_ALIGN_CENTER, -90, 20);
    lv_obj_set_style_bg_color(left_cheek, lv_color_hex(0xFF80AB), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(left_cheek, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_border_width(left_cheek, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(left_cheek, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    
    // 頬の赤み (右)
    lv_obj_t *right_cheek = lv_obj_create(face_circle);
    lv_obj_set_size(right_cheek, 50, 40);
    lv_obj_align(right_cheek, LV_ALIGN_CENTER, 90, 20);
    lv_obj_set_style_bg_color(right_cheek, lv_color_hex(0xFF80AB), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(right_cheek, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_border_width(right_cheek, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(right_cheek, LV_RADIUS_CIRCLE, LV_PART_MAIN);
}

// ===== まばたきアニメーション =====
void blink_animation() {
    static int blink_phase = 0;
    
    if (blink_phase == 0) {
        // 目を閉じる
        lv_obj_set_height(left_eye, 10);
        lv_obj_set_height(right_eye, 10);
        blink_phase = 1;
    } else {
        // 目を開ける
        lv_obj_set_height(left_eye, 65);
        lv_obj_set_height(right_eye, 65);
        blink_phase = 0;
        blink_timer = millis() + random(2000, 5000);
    }
}

// ===== 話すアニメーション =====
void talk_animation() {
    static bool mouth_open = false;
    
    if (mouth_open) {
        lv_obj_set_size(mouth, 30, 15);
    } else {
        lv_obj_set_size(mouth, 40, 35);
    }
    mouth_open = !mouth_open;
}

// ===== 驚きアニメーション =====
void surprise_animation() {
    lv_obj_set_size(left_eye, 55, 75);
    lv_obj_set_size(right_eye, 55, 75);
    lv_obj_set_size(mouth, 45, 45);
}

// ===== 通常状態に戻す =====
void reset_to_idle() {
    lv_obj_set_size(left_eye, 45, 65);
    lv_obj_set_size(right_eye, 45, 65);
    lv_obj_set_size(mouth, 30, 15);
    current_anim = ANIM_IDLE;
}

// ===== アニメーション更新 =====
void update_animation() {
    uint32_t current_time = millis();
    
    // 自動まばたき
    if (current_anim == ANIM_IDLE && current_time > blink_timer) {
        blink_animation();
    }
    
    // 話すアニメーション
    if (current_anim == ANIM_TALK) {
        if (current_time - last_anim_time > 150) {
            talk_animation();
            last_anim_time = current_time;
        }
    }
    
    // 2秒後にアイドルに戻る
    if (current_anim != ANIM_IDLE && current_time - last_anim_time > 2000) {
        reset_to_idle();
    }
}

// ===== オーディオ初期化 =====
void audio_init() {
    // I2S設定 (PCM5101A用)
    audio.setPinout(I2S_BCLK, I2S_LRCK, I2S_DOUT);
    audio.setVolume(15); // 0-21
}

// ===== かわいい声でしゃべる (TTS風の効果音) =====
void speak_cute() {
    current_anim = ANIM_TALK;
    last_anim_time = millis();
    
    // ここでは簡単なビープ音を生成
    // 実際のTTSまたは録音済み音声ファイルを使用することを推奨
    Serial.println("かわいい声でしゃべります♪");
}

// ===== セットアップ =====
void setup() {
    Serial.begin(115200);
    Serial.println("ESP32-S3-Touch-LCD-1.85 カービィ風キャラクター起動中...");
    
    // LCD背面照明
    pinMode(LCD_BL, OUTPUT);
    digitalWrite(LCD_BL, HIGH);
    
    // SPI初期化
    SPI.begin(LCD_SCLK, LCD_MISO, LCD_MOSI);
    
    // I2C初期化 (タッチとセンサー用)
    Wire.begin(TOUCH_SDA, TOUCH_SCL);
    
    // LVGL初期化
    lvgl_init();
    
    // キャラクター作成
    create_kirby_character();
    
    // オーディオ初期化
    audio_init();
    
    // まばたきタイマー初期化
    blink_timer = millis() + random(2000, 5000);
    
    Serial.println("初期化完了!");
}

// ===== メインループ =====
void loop() {
    // LVGL更新
    lv_timer_handler();
    
    // アニメーション更新
    update_animation();
    
    // シリアルコマンドでテスト
    if (Serial.available()) {
        char cmd = Serial.read();
        switch (cmd) {
            case 'b': // まばたき
                blink_animation();
                break;
            case 't': // 話す
                speak_cute();
                break;
            case 's': // 驚き
                surprise_animation();
                current_anim = ANIM_SURPRISE;
                last_anim_time = millis();
                break;
            case 'r': // リセット
                reset_to_idle();
                break;
        }
    }
    
    delay(5);
}