/**
 * Touch Driver for ESP32-S3-Touch-LCD-1.85
 * CST816S Capacitive Touch Controller
 */

#ifndef TOUCH_DRIVER_H
#define TOUCH_DRIVER_H

#include <Arduino.h>
#include <Wire.h>

// Touch I2C
#define TOUCH_SDA  4
#define TOUCH_SCL  5
#define TOUCH_INT  7
#define TOUCH_RST  8
#define TOUCH_ADDR 0x15

// Touch info
struct TouchPoint {
    uint16_t x;
    uint16_t y;
    bool touched;
    uint8_t gesture;
};

// Gesture definitions
#define GESTURE_NONE         0x00
#define GESTURE_SWIPE_UP     0x01
#define GESTURE_SWIPE_DOWN   0x02
#define GESTURE_SWIPE_LEFT   0x03
#define GESTURE_SWIPE_RIGHT  0x04
#define GESTURE_SINGLE_CLICK 0x05
#define GESTURE_DOUBLE_CLICK 0x0B
#define GESTURE_LONG_PRESS   0x0C

class TouchDriver {
private:
    TwoWire* wire;
    TouchPoint current_point;
    
public:
    TouchDriver();
    ~TouchDriver();
    
    bool init();
    bool read();
    TouchPoint getPoint();
    
    bool isTouched();
    uint16_t getX();
    uint16_t getY();
    uint8_t getGesture();
    
    // Callback for touch events
    typedef void (*TouchCallback)(TouchPoint point);
    void setTouchCallback(TouchCallback callback);
    
private:
    TouchCallback touch_callback;
    void reset();
    uint8_t readRegister(uint8_t reg);
    void writeRegister(uint8_t reg, uint8_t value);
};

#endif
