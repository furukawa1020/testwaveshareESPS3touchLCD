/**
 * Display Driver for ESP32-S3-Touch-LCD-1.85
 * GC9A01 360x360 Round LCD
 */

#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <lvgl.h>

// Display pins
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   10
#define TFT_DC   11
#define TFT_RST  12
#define TFT_BL   45

// Display size
#define SCREEN_WIDTH  360
#define SCREEN_HEIGHT 360

class DisplayDriver {
private:
    TFT_eSPI* tft;
    uint8_t brightness;
    
public:
    DisplayDriver();
    ~DisplayDriver();
    
    bool init();
    void setBrightness(uint8_t level);
    uint8_t getBrightness();
    
    void fillScreen(uint16_t color);
    void clearScreen();
    
    TFT_eSPI* getTFT() { return tft; }
    
    // LVGL flush callback
    static void lvgl_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
    
private:
    void setupBacklight();
};

#endif
