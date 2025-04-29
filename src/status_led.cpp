#include "status_led.h"

StatusLED::StatusLED(uint8_t numPixels, uint8_t pin, uint8_t brightness)
    : led(numPixels, pin, NEO_GRB + NEO_KHZ800), _brightness(brightness), _numPixels(numPixels), _ledState(false), _blinkState(false) {
}

void StatusLED::begin() {
    led.begin();
    led.setBrightness(_brightness);
    setColor(0, 0, 0);
    led.show(); // Turn off all pixels initially
}

void StatusLED::loop() {
    blinkLoop();
}

void StatusLED::configureColor(const char *color) {
    // Check if string is formatted in hex "#123456" (7 digits)
    if (strlen(color) == 7) {
        LOG.info(String("Update StatusLED color to " + String(color)));
        // extract red, green, blue values
        uint8_t red, green, blue;
        hexToRgb(color, red, green, blue);
        if (_red == red && _green == green && _blue == blue) {
            // No changes
            return;
        } else {
            _red = red;
            _green = green;
            _blue = blue;
            LOG.info("StatusLED: Applied new color configuration");
            LOG.debug(String(red) + ", " + String(green) + ", " + String(blue));

            // if led is turned on display changes on status led
            if (_ledState) {
                setColor(red, green, blue);
                led.show();
            }
        }
    } else {
        LOG.warning("[CL-CM-01] Invalid Command Format");
    }
}

void StatusLED::configureColor(uint8_t red, uint8_t green, uint8_t blue) {
    // store color information
    _red = red;
    _green = green;
    _blue = blue;

    if (_ledState) { // update color if led is on
        setColor(_red, _green, _blue);
        led.show();
    }
}

void StatusLED::turnOn() {
    _ledState = true;
    LOG.info("StatusLED turn on");
    if (!_blinkState) { // only turn on led immediately if it is not in blink state
        setColor(_red, _green, _blue);
        led.show();
    }
}

void StatusLED::turnOff() {
    _ledState = false;
    LOG.info("StatusLED turn off");
    setColor(0, 0, 0);
    led.show();
}

void StatusLED::startBlinking() {
    _blinkState = true;
    LOG.info("StatusLED start blinking");
    if (_ledState) { // If led turns from state on to state blink
        setColor(0, 0, 0);
    } else { // If led turns from state off to state blink
        setColor(_red, _green, _blue);
    }
}

void StatusLED::stopBlinking() {
    _blinkState = false;
    LOG.info("StatusLED stop blinking");
    if (_ledState) { // If led turn from blink to on, turn led on
        setColor(_red, _green, _blue);
        led.show();
    }
}

void StatusLED::blinkLoop() {
    if (millis() - _previousMillisBlink >= BLINK_INTERVALE / 4) { // works also when millis() rollover, because durations are compared
        _previousMillisBlink = millis();
        _generalBlinkState = !_generalBlinkState;

        if (_blinkState && _ledState) {
            if (_generalBlinkState == true) {
                setColor(_red, _green, _blue); // Full
            } else
                setColor(0, 0, 0);
            led.show();
        }
    }
}

void StatusLED::setColor(uint8_t red, uint8_t green, uint8_t blue) {
    for (int i = 0; i < _numPixels; i++) {
        led.setPixelColor(i, red, green, blue);
    }
}

void StatusLED::hexToRgb(const char *hex, uint8_t &red, uint8_t &green,
                         uint8_t &blue) {
    unsigned int r, g, b;
    sscanf(hex, "#%02x%02x%02x", &r, &g, &b);
    red = static_cast<uint8_t>(r);
    green = static_cast<uint8_t>(g);
    blue = static_cast<uint8_t>(b);
}