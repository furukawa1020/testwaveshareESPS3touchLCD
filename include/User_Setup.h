// TFT_eSPI User Setup for ESP32-S3-Touch-LCD-1.85
// This file configures TFT_eSPI for the Waveshare round display

#ifndef USER_SETUP_H
#define USER_SETUP_H

// Driver selection
#define GC9A01_DRIVER      // Round display driver

// Pin definitions for ESP32-S3-Touch-LCD-1.85
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   10
#define TFT_DC   11
#define TFT_RST  12
#define TFT_BL   45

// Display resolution
#define TFT_WIDTH  360
#define TFT_HEIGHT 360

// Fonts
#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH
#define LOAD_GFXFF  // FreeFonts

#define SMOOTH_FONT

// SPI frequency
#define SPI_FREQUENCY  40000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000

// Optional reduced SPI frequency for reading TFT
#define SPI_READ_FREQUENCY  20000000

#endif
