#include "touch_driver.h"

TouchDriver::TouchDriver() {
    wire = &Wire;
    touch_callback = nullptr;
    current_point = {0, 0, false, GESTURE_NONE};
}

TouchDriver::~TouchDriver() {
}

bool TouchDriver::init() {
    Serial.println("タッチスクリーン初期化中...");
    
    // Initialize I2C
    wire->begin(TOUCH_SDA, TOUCH_SCL);
    wire->setClock(400000);  // 400kHz
    
    // Reset touch controller
    reset();
    delay(100);
    
    // Check if touch controller is responding
    wire->beginTransmission(TOUCH_ADDR);
    if (wire->endTransmission() != 0) {
        Serial.println("タッチコントローラーが見つかりません!");
        return false;
    }
    
    Serial.println("タッチスクリーン初期化完了!");
    return true;
}

void TouchDriver::reset() {
    pinMode(TOUCH_RST, OUTPUT);
    digitalWrite(TOUCH_RST, LOW);
    delay(10);
    digitalWrite(TOUCH_RST, HIGH);
    delay(50);
}

bool TouchDriver::read() {
    wire->beginTransmission(TOUCH_ADDR);
    wire->write(0x02);  // Touch data register
    if (wire->endTransmission() != 0) {
        return false;
    }
    
    // Read 6 bytes: status, x_high, x_low, y_high, y_low, gesture
    wire->requestFrom(TOUCH_ADDR, 6);
    if (wire->available() < 6) {
        return false;
    }
    
    uint8_t data[6];
    for (int i = 0; i < 6; i++) {
        data[i] = wire->read();
    }
    
    // Parse touch data
    uint8_t points = data[0] & 0x0F;
    current_point.touched = (points > 0);
    
    if (current_point.touched) {
        current_point.x = ((data[1] & 0x0F) << 8) | data[2];
        current_point.y = ((data[3] & 0x0F) << 8) | data[4];
        current_point.gesture = data[5];
        
        // Call callback if set
        if (touch_callback) {
            touch_callback(current_point);
        }
    } else {
        current_point.gesture = GESTURE_NONE;
    }
    
    return true;
}

TouchPoint TouchDriver::getPoint() {
    return current_point;
}

bool TouchDriver::isTouched() {
    return current_point.touched;
}

uint16_t TouchDriver::getX() {
    return current_point.x;
}

uint16_t TouchDriver::getY() {
    return current_point.y;
}

uint8_t TouchDriver::getGesture() {
    return current_point.gesture;
}

void TouchDriver::setTouchCallback(TouchCallback callback) {
    touch_callback = callback;
}

uint8_t TouchDriver::readRegister(uint8_t reg) {
    wire->beginTransmission(TOUCH_ADDR);
    wire->write(reg);
    wire->endTransmission();
    
    wire->requestFrom(TOUCH_ADDR, 1);
    if (wire->available()) {
        return wire->read();
    }
    return 0;
}

void TouchDriver::writeRegister(uint8_t reg, uint8_t value) {
    wire->beginTransmission(TOUCH_ADDR);
    wire->write(reg);
    wire->write(value);
    wire->endTransmission();
}
