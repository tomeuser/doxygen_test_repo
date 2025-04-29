/**************************************************************************/
/**
 * @file status_led.h
 * @author Daniel Ahlers (d.ahlers@adunas.dev)
 * @brief This is the class to control the status led
 * @version 0.1
 * @date 2025-02-03
 *
 * @copyright Copyright (c) 2024, Adunas GmbH
 *            All rights reserved.
 *            Unauthorized copying of this file, via any medium is strictly
 *            prohibited Proprietary and confidential *
 */
/**************************************************************************/

#ifndef STATUS_LED_H
#define STATUS_LED_H

#include "logger.h"
#include <Adafruit_NeoPixel.h>

class StatusLED {
public:
    StatusLED(uint8_t numPixels = 1, uint8_t pin = -1, uint8_t brightness = 255);
    void begin();
    void loop();
    void configureColor(const char *color);
    void configureColor(uint8_t red, uint8_t green, uint8_t blue);
    void turnOn();
    void turnOff();
    void startBlinking();
    void stopBlinking();

private:
    void blinkLoop();
    void setColor(uint8_t red, uint8_t green, uint8_t blue);
    void hexToRgb(const char *hex, uint8_t &red, uint8_t &green, uint8_t &blue);

    Adafruit_NeoPixel led;
    uint8_t _brightness;
    uint8_t _numPixels;

    // Store current settings
    uint8_t _red;
    uint8_t _green;
    uint8_t _blue;
    bool _ledState;
    bool _blinkState;

    // Blink
    unsigned long _previousMillisBlink = 0;
    bool _generalBlinkState = false;
};

#endif // STATUS_LED_H
