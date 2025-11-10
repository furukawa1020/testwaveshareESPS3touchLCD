#include "display_driver.h"

// Global TFT instance for LVGL callback
static TFT_eSPI* g_tft = nullptr;

DisplayDriver::DisplayDriver() {
    tft = new TFT_eSPI(SCREEN_WIDTH, SCREEN_HEIGHT);
    g_tft = tft;
    brightness = 255;
}

DisplayDriver::~DisplayDriver() {
    if (tft) {
        delete tft;
    }
}

bool DisplayDriver::init() {
    Serial.println("ディスプレイ初期化中...");
    
    // Backlight setup
    setupBacklight();
    
    // TFT initialization
    tft->init();
    tft->setRotation(0);  // Portrait mode
    tft->fillScreen(TFT_BLACK);
    
    // Set brightness
    setBrightness(brightness);
    
    Serial.println("ディスプレイ初期化完了!");
    return true;
}

void DisplayDriver::setupBacklight() {
    pinMode(TFT_BL, OUTPUT);
    
    // Setup PWM for backlight control
    ledcSetup(0, 5000, 8);  // Channel 0, 5kHz, 8-bit resolution
    ledcAttachPin(TFT_BL, 0);
}

void DisplayDriver::setBrightness(uint8_t level) {
    brightness = level;
    ledcWrite(0, brightness);
}

uint8_t DisplayDriver::getBrightness() {
    return brightness;
}

void DisplayDriver::fillScreen(uint16_t color) {
    tft->fillScreen(color);
}

void DisplayDriver::clearScreen() {
    tft->fillScreen(TFT_BLACK);
}

void DisplayDriver::lvgl_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    if (!g_tft) {
        lv_disp_flush_ready(disp);
        return;
    }
    
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    
    g_tft->startWrite();
    g_tft->setAddrWindow(area->x1, area->y1, w, h);
    g_tft->pushColors((uint16_t *)&color_p->full, w * h, true);
    g_tft->endWrite();
    
    lv_disp_flush_ready(disp);
}
