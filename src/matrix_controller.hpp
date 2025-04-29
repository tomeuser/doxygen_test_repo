/**************************************************************************/
/**
 * @file matrix_controller.hpp
 * @author Tom Meuser (t.meuser@innovaze-media.com)
 * @brief This file defines all methods of the template class MatrixController which were declared
 * in matrix_controller.h
 * @version 0.1
 * @date 2024-09-19
 *
 * @copyright Copyright (c) 2024, Adunas GmbH
 *            All rights reserved.
 *            Unauthorized copying of this file, via any medium is strictly
 *            prohibited Proprietary and confidential *
 */
/**************************************************************************/

#include "matrix_controller.h"

/**************************************************************************/
/**
 * @brief Constructor to initialize the LED matrix.
 *
 * @param chamberWidth An array representing the width of each chamber.
 * @param chamberHeight The height of the chambers.
 * @param pin GPIO pin to control the LED matrix.
 * @param mqttClient Pointer to MqttClient for sending debug messages.
 * @param lightSensor Pointer to LightSensor for controlling matrix brightness.
 * @param ioMultiplexer Pointer to IO multiplexer (optional, depends on ACK_BUTTON)
 *
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
MatrixController<chamberCount>::MatrixController(const uint8_t (&chamberWidth)[chamberCount], uint8_t chamberHeight, uint8_t pin, MqttClient *mqttClient, LightSensor<LED_MATRIX_COUNT> *lightSensor, PCA9535 *ioMultiplexer)
    : Adafruit_NeoPixel(LED_MATRIX_WIDTH * LED_MATRIX_HEIGHT * LED_MATRIX_COUNT + 1, pin, NEO_GRB + NEO_KHZ800) {
    for (int i = 0; i < chamberCount; i++) {
        _chamberWidth[i] = chamberWidth[i];
    }
    _chamberHeight = chamberHeight;
    _mqttClient = mqttClient;
    _lightSensor = lightSensor;
#ifdef ACK_BUTTON
    _ioMultiplexer = ioMultiplexer;
#endif
    pinMode(MATRIX_POWER_PIN, OUTPUT);
}

/**************************************************************************/
/**
 * @brief Constructor to initialize the LED matrix.
 *
 * @param chamberWidth An array representing the width of each chamber.
 * @param chamberHeight The height of the chambers.
 * @param pin GPIO pin to control the LED matrix.
 * @param mqttClient Pointer to MqttClient for sending debug messages.
 * @param lightSensor Pointer to LightSensor for controlling matrix brightness
 *
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
MatrixController<chamberCount>::MatrixController(const uint8_t (&chamberWidth)[chamberCount], uint8_t chamberHeight, uint8_t pin, MqttClient *mqttClient, LightSensor<LED_MATRIX_COUNT> *lightSensor)
    : Adafruit_NeoPixel(LED_MATRIX_WIDTH * LED_MATRIX_HEIGHT * LED_MATRIX_COUNT + 1, pin, NEO_GRB + NEO_KHZ800) {
    for (int i = 0; i < chamberCount; i++) {
        _chamberWidth[i] = chamberWidth[i];
    }
    _chamberHeight = chamberHeight;
    _mqttClient = mqttClient;
    _lightSensor = lightSensor;
    pinMode(MATRIX_POWER_PIN, OUTPUT);
}

/**************************************************************************/
/**
 * @brief Initializes the matrix and sets up default settings for all chambers.
 *
 * This function sets the initial state of the matrix, including turning off
 * all chambers and setting default RGB values for each.
 *
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
void MatrixController<chamberCount>::begin() {
    Adafruit_NeoPixel::begin();
    // Iterate through all chambers
    uint8_t red = DEFAULT_RED, green = DEFAULT_GREEN, blue = DEFAULT_BLUE;
    checkLimits(red, green, blue);
    for (int i = 0; i < chamberCount; i++) {
        _matrixState[i] = false;
        _blinkState[i] = false;
        _red[i] = red;
        _green[i] = green;
        _blue[i] = blue;

        // send off command to all leds once off during initialisation
        setColor(i, 0, 0, 0);
        Adafruit_NeoPixel::show();
    }
}

/**************************************************************************/
/**
 * @brief Sets the color for a specific chamber in the matrix.
 *
 * @param chamber The chamber number to set the color.
 * @param red The red component of the color.
 * @param green The green component of the color.
 * @param blue The blue component of the color.
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
void MatrixController<chamberCount>::setColor(int chamber, uint8_t red, uint8_t green, uint8_t blue) {
    if (chamber <= chamberCount) {
        int xStartPixel = 0;
        for (int i = 0; i < chamber; i++) {
            xStartPixel += _chamberWidth[i];
        }
#ifdef ACK_BUTTON
        int startPixel = xStartPixel * _chamberHeight + 1;
#else
        int startPixel = xStartPixel * _chamberHeight;
#endif
        int endPixel = startPixel + _chamberWidth[chamber] * _chamberHeight;

        // Iterates through all pixel of one tile
        for (int x = startPixel; x < endPixel; x++) {
            Adafruit_NeoPixel::setPixelColor(x, pgm_read_byte(&gamma8[red]), pgm_read_byte(&gamma8[green]), pgm_read_byte(&gamma8[blue]));
        }
    }
}

/**************************************************************************/
/**
 * @brief Resets the color and blink settings of a specific chamber.
 *
 * This function turns off the specified chamber and disables any blinking.
 *
 * @param chamber The chamber number to reset.
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
inline void MatrixController<chamberCount>::reset(int chamber) {
    _blinkState[chamber] = false;
    _matrixState[chamber] = false;
    setColor(chamber, 0, 0, 0);
}

/**************************************************************************/
/**
 * @brief Controls the matrix based on incoming messages.
 *
 * This function processes incoming MQTT messages to control the matrix power
 * for a specific chamber (on/off).
 *
 * @param chamber The chamber number to control.
 * @param incomingMessage A string message that controls the matrix state ("on" or "off").
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
inline void MatrixController<chamberCount>::controlMatrix(int chamber, const char *incomingMessage) {
    if (matrixControl) {
        unsigned long timestampTest = millis();
        String msg = String(incomingMessage);
        if (msg == "on") {
            if (!_matrixState[chamber]) {
                digitalWrite(MATRIX_POWER_PIN, HIGH);
                LOG.debug(String("Turn on matrix with rgb-values r: " + String(_red[chamber]) + " g: " + String(_green[chamber]) + " b: " + String(_blue[chamber])));
                if (!_blinkState[chamber]) {
                    setColor(chamber, _red[chamber], _green[chamber], _blue[chamber]);
                    Adafruit_NeoPixel::show();
#ifndef AD_CL_CL1S
                    _lightSensor->checkPoweredOn(chamber);
#endif
                } else {
                    startBlink(chamber);
                }
                _matrixState[chamber] = true;
                LOG.info(String("Chamber " + String(chamber + 1) + ": Turn On"));
#ifdef ACK_BUTTON
                setLedRing(_red[chamber], _green[chamber], _blue[chamber]);
#endif
            }
        } else if (msg == "off") {
            if (_matrixState[chamber]) {
                if (!_blinkState[chamber]) {
#ifndef AD_CL_CL1S
                    _lightSensor->checkPoweredOff(chamber);
#endif
                }
                setColor(chamber, 0, 0, 0);
                Adafruit_NeoPixel::show();
#ifdef ACK_BUTTON
                setLedRing(0, 0, 0);
#endif
                _matrixState[chamber] = false;
                LOG.info(String("Chamber " + String(chamber + 1) + ": Turn Off"));
            }
        } else {
            LOG.warning("[CL-CM-01] Invalid Command Format");
        }

        // Check if all matrix are turned off to turn power off
        bool isAnyMatrixOn = false;
        for (int i = 0; i < chamberCount; i++) {
            isAnyMatrixOn = (isAnyMatrixOn || _matrixState[i]);
        }
        if (!isAnyMatrixOn) {
            digitalWrite(MATRIX_POWER_PIN, LOW);
        }
        LOG.debug(String("Matrix command processing time: " + String(millis() - timestampTest) + "ms"));
    } else {
        LOG.error("[CL-OV-03] Overheat Protection Engaged");
    }
}

/**************************************************************************/
/**
 * @brief Configures the color of a specific chamber using a hex color string.
 *
 * This function takes a hex string (e.g., "#123456") to update the RGB color of
 * a specified chamber.
 *
 * @param chamber The chamber number to configure.
 * @param color A string representing the new color in hex format (e.g., "#123456").
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
inline void MatrixController<chamberCount>::configureColor(int chamber, const char *color) {
    if (matrixControl) {
        unsigned long timestampTest = millis();
        if (chamber < chamberCount) {
            // Check if string is formatted in hex "#123456" (7 digits)
            if (strlen(color) == 7) {
                LOG.info(String("Chamber " + String(chamber + 1) + ": Update color to " + String(color)));
                // extract red, green, blue values
                uint8_t red, green, blue;
                hexToRgb(color, red, green, blue);
                if (_red[chamber] == red && _green[chamber] == green && _blue[chamber] == blue) {
                    // No changes
                    return;
                } else {
                    checkLimits(red, green, blue);
                    if (_red[chamber] == red && _green[chamber] == green && _blue[chamber] == blue) {
                        return;
                    } else {
                        _red[chamber] = red;
                        _green[chamber] = green;
                        _blue[chamber] = blue;
                        LOG.info("Chamber " + String(chamber + 1) + ": Applied new color configuration");

                        // if matrix is turned on and not blinking, display changes on led matrix
                        if (_matrixState[chamber] && !_blinkState[chamber]) {
                            setColor(chamber, _red[chamber], _green[chamber], _blue[chamber]);
                            Adafruit_NeoPixel::show();
                        }
                    }
                }
            } else {
                LOG.warning("[CL-CM-01] Invalid Command Format");
            }
            LOG.debug(String("Matrix command processing time: " + String(millis() - timestampTest) + "ms"));
        }
    } else {
        LOG.error("[CL-OV-03] Overheat Protection Engaged");
    }
}

/**************************************************************************/
/**
 * @brief Configures the color of a specific chamber using RGB values.
 *
 * This function updates the RGB values of a specified chamber.
 *
 * @param chamber The chamber number to configure.
 * @param red The red component (0-255).
 * @param green The green component (0-255).
 * @param blue The blue component (0-255).
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
inline void MatrixController<chamberCount>::configureColor(int chamber, uint8_t red, uint8_t green, uint8_t blue) {
    if (matrixControl) {
        unsigned long timestampTest = millis();
        if (chamber < chamberCount) {
            // Check if new values change color
            if (_red[chamber] == red && _green[chamber] == green && _blue[chamber] == blue) {
                // No changes
                return;
            } else {
                checkLimits(red, green, blue);
                if (_red[chamber] == red && _green[chamber] == green && _blue[chamber] == blue) {
                    return;
                } else {
                    _red[chamber] = red;
                    _green[chamber] = green;
                    _blue[chamber] = blue;
                    LOG.info("Chamber " + String(chamber + 1) + ": Applied new color configuration");
                    // if matrix is turned on and not blinking, display changes on led matrix
                    if (_matrixState[chamber] && !_blinkState[chamber]) {
                        setColor(chamber, _red[chamber], _green[chamber], _blue[chamber]);
                        Adafruit_NeoPixel::show();
                    }
                }
            }
        }
        LOG.debug(String("Matrix command processing time: " + String(millis() - timestampTest) + "ms"));
    } else {
        LOG.error("[CL-OV-03] Overheat Protection Engaged");
    }
}

/**************************************************************************/
/**
 * @brief Configures the blink state of a specific chamber.
 *
 * This function processes incoming MQTT messages to enable or disable blinking
 * for a specific chamber.
 *
 * @param chamber The chamber number to configure.
 * @param incomingMessage A string message that controls the blink state ("1" / "true" or "0" / "false").
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
inline void MatrixController<chamberCount>::configureBlink(int chamber, const char *incomingMessage) {
    if (matrixControl) {
        String msg = String(incomingMessage);
        if (msg == "0" || msg == "false") { // Stop blink
            if (_blinkState[chamber]) {
                _blinkState[chamber] = 0;
                if (_matrixState[chamber]) {
                    setColor(chamber, _red[chamber], _green[chamber], _blue[chamber]);
                    Adafruit_NeoPixel::show();
                }
                LOG.info("Chamber " + String(chamber + 1) + ": Disabled Blink");
            }
        } else if (msg == "1" || msg == "true") {
            if (!_blinkState[chamber]) {
                _blinkState[chamber] = 1;
                if (_matrixState[chamber]) {
                    startBlink(chamber);
                }
                LOG.info("Chamber " + String(chamber + 1) + ": Enabled Blink");
            }
        } else {
            LOG.warning("[CL-CM-01] Invalid Command Format");
        }
    } else {
        LOG.error("[CL-OV-03] Overheat Protection Engaged");
    }
}

/**************************************************************************/
/**
 * @brief Enables or disables matrix control.
 *
 * This function globally enables or disables control over the matrix, turning
 * off all chambers if disabled.
 *
 * @param b True to enable matrix control, false to disable.
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
inline void MatrixController<chamberCount>::enableMatrixControl(bool b) {
    if (b) {
        matrixControl = true;
    } else {
        matrixControl = false;
        digitalWrite(MATRIX_POWER_PIN, LOW);
        for (int i = 0; i < chamberCount; i++) {
            _matrixState[i] = false;
        }
    }
}

/**************************************************************************/
/**
 * @brief Sets the brightness limit for the LED matrix.
 *
 * This function limits the maximum brightness that the matrix can reach.
 *
 * @param level Brightness limit (0-255).
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
inline void MatrixController<chamberCount>::setLEDBrightnessLimit(u_int8_t level) {
    _brightnessLimit = level;
}

/**************************************************************************/
/**
 * @brief Converts RGB values to HSV values.
 *
 * This function converts RGB (red, green, blue) to HSV (hue, saturation, value) for color
 * manipulations.
 *
 * @param r Red component (0-1.0).
 * @param g Green component (0-1.0).
 * @param b Blue component (0-1.0).
 * @param h Output hue (degrees 0-360).
 * @param s Output saturation (0-1.0).
 * @param v Output value (brightness 0-1.0).
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
inline void MatrixController<chamberCount>::rgb2hsv(float r, float g, float b, float &h, float &s, float &v) {
    double min, max, delta;

    min = r < g ? r : g;
    min = min < b ? min : b;

    max = r > g ? r : g;
    max = max > b ? max : b;

    v = max; // v
    delta = max - min;
    if (delta < 0.00001) {
        s = 0;
        h = 0; // undefined, maybe nan?
        return;
    }
    if (max > 0.0) {       // NOTE: if Max is == 0, this divide would cause a crash
        s = (delta / max); // s
    } else {
        // if max is 0, then r = g = b = 0
        // s = 0, h is undefined
        s = 0.0;
        h = NAN; // its now undefined
        return;
    }
    if (r >= max)
        h = (g - b) / delta; // between yellow & magenta
    else if (g >= max)
        h = 2.0 + (b - r) / delta; // between cyan & yellow
    else
        h = 4.0 + (r - g) / delta; // between magenta & cyan

    h *= 60.0; // degrees

    if (h < 0.0)
        h += 360.0;
}

/**************************************************************************/
/**
 * @brief Converts HSV values to RGB values.
 *
 * This function converts HSV (hue, saturation, value) to RGB (red, green, blue) for color
 * manipulations.
 *
 * @param r Output red component (0-1.0).
 * @param g Output green component (0-1.0).
 * @param b Output blue component (0-1.0).
 * @param h Input hue (degrees 0-360).
 * @param s Input saturation (0-1.0).
 * @param v Input value (brightness 0-1.0).
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
inline void MatrixController<chamberCount>::hsv2rgb(float &r, float &g, float &b, float h, float s, float v) {
    double hh, p, q, t, ff;
    long i;

    if (s <= 0.0) {
        r = v;
        g = v;
        b = v;
        return;
    }
    hh = h;
    if (hh >= 360.0)
        hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = v * (1.0 - s);
    q = v * (1.0 - (s * ff));
    t = v * (1.0 - (s * (1.0 - ff)));

    switch (i) {
    case 0:
        r = v;
        g = t;
        b = p;
        break;
    case 1:
        r = q;
        g = v;
        b = p;
        break;
    case 2:
        r = p;
        g = v;
        b = t;
        break;

    case 3:
        r = p;
        g = q;
        b = v;
        break;
    case 4:
        r = t;
        g = p;
        b = v;
        break;
    case 5:
    default:
        r = v;
        g = p;
        b = q;
        break;
    }
}

/**************************************************************************/
/**
 * @brief Scales the brightness of an RGB color using the HSV model.
 *
 * This function scales the brightness of a given RGB color by converting it to HSV,
 * adjusting the value (brightness), and converting it back to RGB.
 *
 * @param brightness Desired brightness (0-100%).
 * @param red Red component (0-255).
 * @param green Green component (0-255).
 * @param blue Blue component (0-255).
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
inline void MatrixController<chamberCount>::scaleRGB(uint8_t brightness, uint8_t &red, uint8_t &green, uint8_t &blue) {
    float h, s, v;
    float r = (float)red / 255, g = (float)green / 255, b = (float)blue / 255;
    rgb2hsv(r, g, b, h, s, v);
    v = (float)brightness / 100;
    hsv2rgb(r, g, b, h, s, v);
    red = static_cast<uint8_t>(std::max(0.0f, std::min(255.0f, r * 255)));
    green = static_cast<uint8_t>(std::max(0.0f, std::min(255.0f, g * 255)));
    blue = static_cast<uint8_t>(std::max(0.0f, std::min(255.0f, b * 255)));
}

/**************************************************************************/
/**
 * @brief Converts a hex color string to RGB values.
 *
 * This function splits a hex color value into its individual red, green, and blue components.
 *
 * @param hex Hex color value (e.g., "#123456").
 * @param red Output red component (0-255).
 * @param green Output green component (0-255).
 * @param blue Output blue component (0-255).
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
void MatrixController<chamberCount>::hexToRgb(const char *hex, uint8_t &red, uint8_t &green,
                                              uint8_t &blue) {
    unsigned int r, g, b;
    sscanf(hex, "#%02x%02x%02x", &r, &g, &b);
    red = static_cast<uint8_t>(r);
    green = static_cast<uint8_t>(g);
    blue = static_cast<uint8_t>(b);
}

/**************************************************************************/
/**
 * @brief Extracts the brightness from an RGB color.
 *
 * This function calculates the brightness of a color by converting it from RGB to HSV
 * and returning the value (brightness) component.
 *
 * @param red Red component (0-255).
 * @param green Green component (0-255).
 * @param blue Blue component (0-255).
 * @return Brightness (0-100%).
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
uint8_t MatrixController<chamberCount>::extractBrightness(uint8_t red, uint8_t green, uint8_t blue) {
    float r = (float)red / 255, g = (float)green / 255, b = (float)blue / 255;
    double max;
    max = r > g ? r : g;
    max = max > b ? max : b;
    float brightness = (float)max;
    return static_cast<uint8_t>(std::max(0, std::min(100, int(round(brightness * 100)))));
}

/**************************************************************************/
/**
 * @brief Checks if the color is within the defined brightness limits.
 *
 * This function checks if the RGB values for a color exceed the brightness limit and
 * adjusts them if necessary.
 *
 * @param r Red component (0-255).
 * @param g Green component (0-255).
 * @param b Blue component (0-255).
 * @return True if adjustments were made, false otherwise.
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
inline bool MatrixController<chamberCount>::checkLimits(uint8_t &r, uint8_t &g, uint8_t &b) {
    // Check which of the 3 led has the brightest value = max power
    uint8_t highestValue = std::max(r, std::max(g, b));
    uint8_t brightness = extractBrightness(r, g, b);
    float step = 20.0; // Start step
    bool changedDirection = false;

    if (highestValue > _brightnessLimit) {
        while (highestValue > _brightnessLimit || highestValue < _brightnessLimit - 5) {

            if (highestValue > _brightnessLimit) {
                brightness -= (int)round(step); // Decrease brightness
                scaleRGB(brightness, r, g, b);
                highestValue = std::max(r, std::max(g, b));
                if (highestValue < _brightnessLimit - 5) {
                    if (step > 1.0)
                        step /= 2.0;
                }
            } else {
                brightness += (int)round(step); // Increase brightness
                scaleRGB(brightness, r, g, b);
                highestValue = std::max(r, std::max(g, b));
                if (highestValue > _brightnessLimit) {
                    if (step > 1.0)
                        step /= 2.0;
                }
            }
        }
        return true;
    }
    return false;
}

/**************************************************************************/
/**
 * @brief Handles the blink effect for the matrix.
 *
 * This function toggles the brightness of chambers to create a blinking effect.
 * It is called in the main loop of the program.
 *
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
inline void MatrixController<chamberCount>::blinkLoop() {
    if (millis() - previousMillisBlink >= BLINK_INTERVALE / 2) { // works also when millis() rollover, because durations are compared
        _generalBlinkState = !_generalBlinkState;
        // Iterate through all chambers
        for (int i = 0; i < chamberCount; i++) {
            if (_blinkState[i] && _matrixState[i]) {
                if (_generalBlinkState == true) {
                    setColor(i, _red[i], _green[i], _blue[i]); // Full
                } else
                    setColor(i, 0, 0, 0);
                Adafruit_NeoPixel::show();
            }
        }
        previousMillisBlink = millis();
    }
}

/**************************************************************************/
/**
 * @brief Starts the blinking effect for a specific chamber.
 *
 * This function enables the blinking effect for the specified chamber.
 *
 * @param chamber The chamber number to start blinking.
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
inline void MatrixController<chamberCount>::startBlink(int chamber) {
    bool isAnyBlink = false;
    // Check if any other chamber is in blink state
    for (int i = 0; i < chamberCount; i++) {
        if (i != chamber) {
            if (_blinkState[i]) {
                isAnyBlink = true;
            }
        }
    }
    if (!_matrixState[chamber]) { // If matrix turns from state off to state blink
        setColor(chamber, _red[chamber], _green[chamber], _blue[chamber]);
    } else { // If matrix turn from state on to state blink
        setColor(chamber, 0, 0, 0);
    }

    Adafruit_NeoPixel::show();

    if (!isAnyBlink) { // If other chamber are not in blink state, reset blink loop
        if (!_matrixState[chamber]) {
            _generalBlinkState = true;
        } else {
            _generalBlinkState = false;
        }
        previousMillisBlink = millis();
    }
}

/**************************************************************************/
/**
 * @brief Stops the blinking effect for a specific chamber.
 *
 * This function disables the blinking effect and restores the chamber's original color.
 *
 * @param chamber The chamber number to stop blinking.
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
inline void MatrixController<chamberCount>::stopBlink(int chamber) {
    _blinkState[chamber] = 0;
    setColor(chamber, _red[chamber], _green[chamber], _blue[chamber]); // Full
}

/**************************************************************************/
/**
 * @brief Main loop to handle blinking and button states.
 *
 * This function should be called in the main program loop to manage the blink effect
 * and button state changes.
 *
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
inline void MatrixController<chamberCount>::loop() {
    blinkLoop();
#ifdef ACK_BUTTON
    buttonLoop();
#endif
}

#ifdef ACK_BUTTON
/**************************************************************************/
/**
 * @brief Handles the button state and publishes the state via MQTT.
 *
 * This function debounces the acknowledgment button, checks for state changes,
 * and publishes the button state ("0" for released, "1" for pressed) via MQTT.
 *
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
inline void MatrixController<chamberCount>::buttonLoop() {
    // a duration is used here to make the behaviour of the debounce algorithm independent of the frequency buttonLoop() is called
    unsigned long currentMillis = millis();                       // Get the current time
    if (currentMillis - previousMillisButton >= READ_INTERVALE) { // works also when millis() rollover, because durations are compared

        // read button pin and debounce it
        unsigned int currentState = 2; /* Cleaned-up version of the input signal */
        if (_ioMultiplexer->read(PCA95x5::Port::Port(ACK_BUTTON_PIN)) == PCA95x5::Level::L) {
            debounce(false, &_integrator, &currentState);
        } else {
            debounce(true, &_integrator, &currentState);
        }
        if (currentState == 0 || currentState == 1) {
            if (currentState != _buttonState) { // the register state is always one ahead of the button state, because it needs one cycle to initiate
                _buttonState = currentState;

                // Send button state only if matrix is active
                if (_buttonState == 0) {
                    LOG.info("Button Pressed");
                    if (_matrixState[0]) {
                        _mqttClient->publish(_topic.c_str(), "1", true);
                        turnLedOff = true;
                    }
                }

                else {
                    LOG.info("Button Released");
                    if (_matrixState[0]) {
                        _mqttClient->publish(_topic.c_str(), "0", true);
                        if (turnLedOff) {
                            setLedRing(0, 0, 0);
                            turnLedOff = false;
                        }
                    }
                }
            }
        }
        previousMillisButton = currentMillis; // Reset the  timer
    }
}

/**************************************************************************/
/**
 * @brief Software debounce for the acknowledgment button.
 *
 * This function implements a debounce algorithm to avoid reading false signals
 * from the button due to mechanical bounce.
 *
 * @param input The current raw state of the button (true or false).
 * @param integrator Pointer to the debounce integrator.
 * @param output Pointer to the debounced button state (0 or 1).
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
inline void MatrixController<chamberCount>::debounce(bool input, unsigned int *integrator, unsigned int *output) {
    // Based on the integrator algorithm by Kenneth A. Kuhn
    // https://www.kennethkuhn.com/electronics/debounce.c

    // real signal 0000111111110000000111111100000000011111111110000000000111111100000
    // corrupted   0100111011011001000011011010001001011100101111000100010111011100010
    // integrator  0100123233233212100012123232101001012321212333210100010123233321010
    // output      0000001111111111100000001111100000000111111111110000000001111111000

    if (input == 0) {
        if (*integrator > 0)
            *integrator = (*integrator - 1);
    } else if (*integrator < MAXIMUM_DEBOUNCE_COUNTER)
        *integrator = (*integrator + 1);

    if (*integrator == 0)
        *output = 0;
    else if (*integrator >= MAXIMUM_DEBOUNCE_COUNTER) {
        *integrator = MAXIMUM_DEBOUNCE_COUNTER; /* defensive code if integrator got corrupted */
        *output = 1;
    }
}

/**************************************************************************/
/**
 * @brief Initializes the matrix controller with the MQTT topic for button state.
 *
 * This function sets up the matrix and prepares it to publish button state changes
 * via MQTT to the provided topic.
 *
 * @param topic The MQTT topic to publish button state changes.
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
inline void MatrixController<chamberCount>::begin(const char *topic) {
    _topic = topic;
    Adafruit_NeoPixel::begin();
    // Iterate through all chambers
    uint8_t red = DEFAULT_RED, green = DEFAULT_GREEN, blue = DEFAULT_BLUE;
    checkLimits(red, green, blue);
    for (int i = 0; i < chamberCount; i++) {
        _matrixState[i] = false;
        _blinkState[i] = false;
        _red[i] = red;
        _green[i] = green;
        _blue[i] = blue;
    }
}

/**************************************************************************/
/**
 * @brief Controls the LED ring of the acknowledgment button.
 *
 * @param red The red component of the color.
 * @param green The green component of the color.
 * @param blue The blue component of the color.
 * @tparam chamberCount The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
inline void MatrixController<chamberCount>::setLedRing(uint8_t red, uint8_t green, uint8_t blue) {
    Adafruit_NeoPixel::setPixelColor(0, pgm_read_byte(&gamma8[red]), pgm_read_byte(&gamma8[green]), pgm_read_byte(&gamma8[blue]));
    Adafruit_NeoPixel::show();
}
#endif
