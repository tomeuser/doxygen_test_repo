/**************************************************************************/
/**
 * @file matrix_controller.h
 * @author Tom Meuser (t.meuser@innovaze-media.com)
 * @brief This is the header file of the template class MatrixController declaring all variables
 * and methods used by this class
 * @version 0.1
 * @date 2024-09-19
 *
 * @copyright Copyright (c) 2024, Adunas GmbH
 *            All rights reserved.
 *            Unauthorized copying of this file, via any medium is strictly
 *            prohibited Proprietary and confidential *
 */
/**************************************************************************/

#ifndef MATRIX_CONTROLLER_H
#define MATRIX_CONTROLLER_H
#include "light_sensor.h"
#include "logger.h"
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <PCA95x5.h>
#include <mqtt_client.h>

template <int chamberCount>
class MatrixController : private Adafruit_NeoPixel {

public:
    MatrixController(const uint8_t (&chamberWidth)[chamberCount], uint8_t chamberHeight, uint8_t pin, MqttClient *mqttClient, LightSensor<LED_MATRIX_COUNT> *lightSensor, PCA9535 *ioMultiplexer);
    MatrixController(const uint8_t (&chamberWidth)[chamberCount], uint8_t chamberHeight, uint8_t pin, MqttClient *mqttClient, LightSensor<LED_MATRIX_COUNT> *lightSensor);

    void begin();
    void loop();
    void setLEDBrightnessLimit(u_int8_t level); // 0-255
#ifdef ACK_BUTTON
    void begin(const char *topic);
    void setLedRing(uint8_t red, uint8_t green, uint8_t blue);
#endif
    void controlMatrix(int chamber, const char *incomingMessage);
    void configureColor(int chamber, const char *color);
    void configureColor(int chamber, uint8_t red, uint8_t green, uint8_t blue);
    void configureBlink(int chamber, const char *incomingMessage);
    void enableMatrixControl(bool b);

private:
    LightSensor<LED_MATRIX_COUNT> *_lightSensor;
    MqttClient *_mqttClient;
    uint8_t _chamberWidth[chamberCount];
    uint8_t _chamberHeight;

    // Store current settings
    uint8_t _red[chamberCount];
    uint8_t _green[chamberCount];
    uint8_t _blue[chamberCount];

    // Blink
    bool _blinkState[chamberCount];
    bool _generalBlinkState = false;
    unsigned long previousMillisBlink = 0;
    u_int8_t _brightnessLimit = MAX_BRIGHTNESS;
    bool _matrixState[chamberCount];
    bool matrixControl = true;

// Acknowledgment button
#ifdef ACK_BUTTON
    PCA9535 *_ioMultiplexer;
    String _topic;
    unsigned int _buttonState = 1;
    unsigned int _integrator = 0; // button states integrator used in the debouncing algorithm
    unsigned long previousMillisButton = 0;
    void buttonLoop();
    void debounce(bool input, unsigned int *integrator, unsigned int *output);
    bool turnLedOff = false;
#endif

    // Gamma correction 1 byte bit depth
    const uint8_t PROGMEM gamma8[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2,
        2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4,
        4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8,
        8, 9, 9, 9, 10, 10, 10, 11, 11, 11, 12, 12, 13, 13, 13,
        14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21,
        21, 22, 22, 23, 24, 24, 25, 25, 26, 27, 27, 28, 29, 29, 30,
        31, 32, 32, 33, 34, 35, 35, 36, 37, 38, 39, 39, 40, 41, 42,
        43, 44, 45, 46, 47, 48, 49, 50, 50, 51, 52, 54, 55, 56, 57,
        58, 59, 60, 61, 62, 63, 64, 66, 67, 68, 69, 70, 72, 73, 74,
        75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89, 90, 92, 93, 95,
        96, 98, 99, 101, 102, 104, 105, 107, 109, 110, 112, 114, 115, 117, 119,
        120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142, 144, 146,
        148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175, 177,
        180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
        215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252,
        255};

    // color-brightness manipulation
    // source: https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
    void hexToRgb(const char *hex, uint8_t &red, uint8_t &green, uint8_t &blue);
    void rgb2hsv(float r, float g, float b, float &h, float &s, float &v);
    void hsv2rgb(float &r, float &g, float &b, float h, float s, float v);
    void scaleRGB(uint8_t brightness, uint8_t &red, uint8_t &green, uint8_t &blue);
    uint8_t extractBrightness(uint8_t red, uint8_t green, uint8_t blue);
    bool checkLimits(uint8_t &r, uint8_t &g, uint8_t &b);

    // Private control functions
    void blinkLoop();
    void startBlink(int chamber);
    void stopBlink(int chamber);
    void setColor(int chamber, uint8_t red, uint8_t green, uint8_t blue);
    void reset(int chamber);
};

#include "matrix_controller.hpp"
#endif