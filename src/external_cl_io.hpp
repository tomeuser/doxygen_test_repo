/**************************************************************************/
/**
 * @file external_cl_io.hpp
 * @author Tom Meuser (t.meuser@innovaze-media.com)
 * @brief This file defines all methods of the template class ExternalClIO which were declared
 * in external_cl_io.h
 * @version 0.1
 * @date 2024-09-19
 *
 * @copyright Copyright (c) 2024, Adunas GmbH
 *            All rights reserved.
 *            Unauthorized copying of this file, via any medium is strictly
 *            prohibited Proprietary and confidential *
 */
/**************************************************************************/
#include "external_cl_io.h"

/**************************************************************************/
/**
 * @brief Constructor to initialize the LED matrix.
 *

 * @param mqttClient Pointer to MqttClient for sending debug messages.
 * @param relays Pointer to IO multiplexer that's connected to relays
 *
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
ExternalClIO<chamberCount>::ExternalClIO(MqttClient *mqttClient, PCA9535 *externalMultiplexer_1, PCA9535 *externalMultiplexer_2, PowerSensor *powerSensor) {
    _mqttClient = mqttClient;
    _powerSensor = powerSensor;
    _externalMultiplexer[0] = externalMultiplexer_1;
    _externalMultiplexer[1] = externalMultiplexer_2;
    for (int i = 0; i < chamberCount; i++) {
        _matrixState[i] = 0;
        _blinkState[i] = false;
        _previousInputState[chamberCount] = 0;
    }
}

template <int chamberCount>
inline void ExternalClIO<chamberCount>::begin(const char *topic) {
    _topic = topic;
    _externalMultiplexer[0]->attach(Wire, I2C_EXTERNAL_IO_MULTIPLEXER_ADDR_1);
    _externalMultiplexer[1]->attach(Wire, I2C_EXTERNAL_IO_MULTIPLEXER_ADDR_2);

    for (int i = 0; i < EXT_MULTIPLEXER_COUNT; i++) {
        _externalMultiplexer[i]->polarity(PCA95x5::Polarity::ORIGINAL_ALL);
        // Define Outputs
        for (int j = 0; j < EXT_MULTIPLEXER_IO_OUTPUTS; j++) {
            _externalMultiplexer[i]->direction(_output[j], PCA95x5::Direction::OUT);
            _externalMultiplexer[i]->write(_output[j], PCA95x5::Level::L);
        }
        // Define Inputs
        for (int j = 0; j < EXT_MULTIPLEXER_IO_INPUTS; j++) {
            _externalMultiplexer[i]->direction(_input[j], PCA95x5::Direction::IN);
        }

        // Check for connectivity
        LOG.info("Check for external IO Multiplexer " + String(i + 1) + " ...");
        _externalMultiplexer[i]->read();
        if (_externalMultiplexer[i]->i2c_error() != 0) {
            LOG.error("[CL-SN-03] IO Multiplexer Unresponsive");
        } else {
            LOG.info("Found valid IO Multiplexer");
            _foundMultiplexer[i] = true;
        }
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
inline void ExternalClIO<chamberCount>::startBlink(int chamber) {
    int output;
    int multiplexer;
    if (chamber < EXT_MULTIPLEXER_IO_OUTPUTS) {
        output = chamber;
        multiplexer = 0;
    } else {
        output = chamber - EXT_MULTIPLEXER_IO_OUTPUTS;
        multiplexer = 1;
    }
    bool isAnyBlink = false;
    // Check if any other Kammer is in blink state
    for (int i = 0; i < chamberCount; i++) {
        if (i != chamber) {
            if (_blinkState[i]) {
                isAnyBlink = true;
            }
        }
    }
    if (!_matrixState[chamber]) { // If matrix turns from state off to state blink
        _externalMultiplexer[multiplexer]->write(_output[output], PCA95x5::Level::H);
    } else { // If matrix turn from state on to state blink
        _externalMultiplexer[multiplexer]->write(_output[output], PCA95x5::Level::L);
    }

    if (!isAnyBlink) { // If other Kammern are not in blink state, reset blink loop
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
void ExternalClIO<chamberCount>::stopBlink(int chamber) {
    int output;
    int multiplexer;
    if (chamber < EXT_MULTIPLEXER_IO_OUTPUTS) {
        output = chamber;
        multiplexer = 0;
    } else {
        output = chamber - EXT_MULTIPLEXER_IO_OUTPUTS;
        multiplexer = 1;
    }
    _blinkState[chamber] = 0;
    _externalMultiplexer[multiplexer]->write(_output[output], PCA95x5::Level::H);
}

/**************************************************************************/
/**
 * @brief Main loop to handle blinking and button states.
 *
 * This function should be called in the main program loop to manage the blink effect
 *
 *
 * @tparam The number of chambers in the matrix.
 */
/**************************************************************************/
template <int chamberCount>
void ExternalClIO<chamberCount>::loop() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillisBlink >= BLINK_INTERVALE / 2) { // works also when millis() rollover, because durations are compared
        previousMillisBlink = currentMillis;
        _generalBlinkState = !_generalBlinkState;
        // Iterate through all chambers
        for (int i = 0; i < chamberCount; i++) {
            if (_blinkState[i] && _matrixState[i]) {
                int output;
                int multiplexer;
                if (i < EXT_MULTIPLEXER_IO_OUTPUTS) {
                    output = i;
                    multiplexer = 0;
                } else {
                    output = i - EXT_MULTIPLEXER_IO_OUTPUTS;
                    multiplexer = 1;
                }

                if (_generalBlinkState == true) {
                    _externalMultiplexer[multiplexer]->write(_output[output], PCA95x5::Level::H);
                } else {
                    _externalMultiplexer[multiplexer]->write(_output[output], PCA95x5::Level::L);
                }
            }
        }
    }

    if (millis() - inputTimestamp > inputInterval) {
        inputTimestamp = millis();
        // read inputs
        for (int i = 0; i < EXT_MULTIPLEXER_COUNT; i++) { // i = multiplexer counter
            if (_foundMultiplexer[i]) {
                uint16_t currentInputStates = _externalMultiplexer[i]->read();
                if (_externalMultiplexer[i]->i2c_error() != 0) {
                    LOG.error("[CL-SN-03] IO Multiplexer Unresponsive");
                } else {
                    for (int j = 0; j < EXT_MULTIPLEXER_IO_INPUTS; j++) { // j = IO channel counter
                        unsigned int currentState = 2;
                        debounce((currentInputStates & (1 << PCA95x5::Port::Port(_input[j]))), &_integrator[i * EXT_MULTIPLEXER_IO_INPUTS + j], &currentState);

                        if (currentState == 0 || currentState == 1) {
                            if (_previousInputState[i * EXT_MULTIPLEXER_IO_INPUTS + j] != currentState) {
                                _previousInputState[i * EXT_MULTIPLEXER_IO_INPUTS + j] = currentState;
                                if (currentState == 0) { // voltage level low
                                    if (_mqttClient->connected()) {
                                        _mqttClient->publish(String(_topic + "/input_" + String((i * EXT_MULTIPLEXER_IO_INPUTS + j + 1))).c_str(), "1", true);
                                    } else {
                                        _mqttClient->publishOnConnect(String(_topic + "/input_" + String((i * EXT_MULTIPLEXER_IO_INPUTS + j + 1))).c_str(), "1", true);
                                    }
                                    LOG.info("Input " + String((i * EXT_MULTIPLEXER_IO_INPUTS + j + 1)) + ": circuit closed");
                                } else { // voltage level high
                                    if (_mqttClient->connected()) {
                                        _mqttClient->publish(String(_topic + "/input_" + String((i * EXT_MULTIPLEXER_IO_INPUTS + j + 1))).c_str(), "0", true);
                                    } else {
                                        _mqttClient->publishOnConnect(String(_topic + "/input_" + String((i * EXT_MULTIPLEXER_IO_INPUTS + j + 1))).c_str(), "0", true);
                                    }
                                    LOG.info("Input " + String((i * EXT_MULTIPLEXER_IO_INPUTS + j + 1)) + ": circuit opened");
                                }
                            }
                        }
                    }
                }
            }
        }
    }
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
inline void ExternalClIO<chamberCount>::controlMatrix(int chamber, const char *incomingMessage) {
    unsigned long timestampTest = millis();
    String msg = String(incomingMessage);

    int output;
    int multiplexer;
    if (chamber < EXT_MULTIPLEXER_IO_OUTPUTS) {
        output = chamber;
        multiplexer = 0;
    } else {
        output = chamber - EXT_MULTIPLEXER_IO_OUTPUTS;
        multiplexer = 1;
    }

    if (msg == "on") {
        if (!_matrixState[chamber]) {
            if (_powerSensor->powerUnderThreshold()) {

                if (!_blinkState[chamber]) {
                    _externalMultiplexer[multiplexer]->write(_output[output], PCA95x5::Level::H);
                } else {
                    startBlink(chamber);
                }
                _matrixState[chamber] = true;
                LOG.info(String("Output " + String(chamber + 1) + ": Turn On"));
            } else {
                LOG.error("[CL-PW-02] Power Threshold Exceeded");
            }
        }
    } else if (msg == "off") {
        if (_matrixState[chamber]) {
            _externalMultiplexer[multiplexer]->write(_output[output], PCA95x5::Level::L);
            _matrixState[chamber] = false;
            LOG.info(String("Output " + String(chamber + 1) + ": Turn Off"));
        }
    } else {
        LOG.warning("[CL-CM-01] Invalid Command Format");
    }
    LOG.debug(String("Matrix command processing time: " + String(millis() - timestampTest) + "ms"));
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
inline void ExternalClIO<chamberCount>::configureBlink(int chamber, const char *incomingMessage) {
    String msg = String(incomingMessage);
    if (msg == "0" || msg == "false") { // Stop blink
        if (_blinkState[chamber]) {
            _blinkState[chamber] = 0;
            if (_matrixState[chamber]) {
                int output;
                int multiplexer;
                if (chamber < EXT_MULTIPLEXER_IO_OUTPUTS) {
                    output = chamber;
                    multiplexer = 0;
                } else {
                    output = chamber - EXT_MULTIPLEXER_IO_OUTPUTS;
                    multiplexer = 1;
                }
                _externalMultiplexer[multiplexer]->write(_output[output], PCA95x5::Level::H);
            }
            LOG.info("Output" + String(chamber + 1) + ": Disabled Blink");
        }
    } else if (msg == "1" || msg == "true") {
        if (!_blinkState[chamber]) {
            _blinkState[chamber] = 1;
            if (_matrixState[chamber]) {
                if (_powerSensor->powerUnderThreshold()) {
                    startBlink(chamber);
                } else {
                    LOG.error("[DC-PW-02] Power Threshold Exceeded");
                }
            }
            LOG.info("Output" + String(chamber + 1) + ": Enabled Blink");
        }
    } else {
        LOG.warning("[CL-CM-01] Invalid Command Format");
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
inline void ExternalClIO<chamberCount>::debounce(bool input, unsigned int *integrator, unsigned int *output) {
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