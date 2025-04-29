/**************************************************************************/
/**
 * @file power_over_ethernet.cpp
 * @author Tom Meuser (t.meuser@innovaze-media.com)
 * @brief This file defines all methods of class PowerOverEthernet which were declared
 * in power_over_ethernet.h
 * @version 0.1
 * @date 2024-09-19
 *
 * @copyright Copyright (c) 2024, Adunas GmbH
 *            All rights reserved.
 *            Unauthorized copying of this file, via any medium is strictly
 *            prohibited Proprietary and confidential *
 */
/**************************************************************************/
#include "power_over_ethernet.h"

#ifndef AD_CL_CL16
/**************************************************************************/
/**
 * @brief Construct a new PowerOverEthernet object with matrix controller.
 *
 * @param mqttClient Pointer to the MqttClient for MQTT communication.
 * @param ioMultiplexer Pointer to the PCA9535 IO multiplexer to read POE states.
 * @param matrixController Pointer to the MatrixController for lock LED matrix.
 */
/**************************************************************************/
PowerOverEthernet::PowerOverEthernet(MqttClient *mqttClient, PCA9535 *ioMultiplexer, MatrixController<CL_CHAMBER_COUNT> *matrixController) {
    _mqttClient = mqttClient;
    _ioMultiplexer = ioMultiplexer;
    _matrixController = matrixController;
}
#else
/**************************************************************************/
/**
 * @brief Construct a new PowerOverEthernet object.
 *
 * @param mqttClient Pointer to the MqttClient for MQTT communication.
 * @param ioMultiplexer Pointer to the PCA9535 IO multiplexer to read POE states.
 */
/**************************************************************************/
PowerOverEthernet::PowerOverEthernet(MqttClient *mqttClient, PCA9535 *ioMultiplexer, PowerSensor *powerSensor) {
    _mqttClient = mqttClient;
    _ioMultiplexer = ioMultiplexer;
    _powerSensor = powerSensor;
}
#endif

/**************************************************************************/
/**
 * @brief This method reads periodically the state of the IO multiplexer
 * to determine the current PoE type and executes the relevant dependencies based on
 * the type
 */
/**************************************************************************/
void PowerOverEthernet::init() {
    uint8_t currentPOEState;

    // set the input low, before checking them
    PCA95x5::Port::Port _input[3] = {PCA95x5::Port::P01, PCA95x5::Port::P02, PCA95x5::Port::P03};
    for (int i = 0; i < 3; i++) {
        _ioMultiplexer->direction(_input[i], PCA95x5::Direction::OUT);
        _ioMultiplexer->write(_input[i], PCA95x5::Level::L);
        _ioMultiplexer->direction(_input[i], PCA95x5::Direction::IN);
    }

    uint16_t ioStates = _ioMultiplexer->read();

    if (_ioMultiplexer->i2c_error() == 0) {

        bool atDet = (ioStates & (1 << 0)) != 0; // IO0_0
        bool t3d = (ioStates & (1 << 1)) != 0;   // IO0_1
        bool t4d = (ioStates & (1 << 2)) != 0;   // IO0_2

        if (!t3d && !t4d) { // 30W POE Module
            if (atDet) {    // POE
                currentPOEState = 1;
            } else { // POE+ or Higher
                currentPOEState = 2;
            }

        } else {                       // 85W POE Module
            if (atDet && t3d && t4d) { // POE
                currentPOEState = 1;
            } else if (!atDet && t3d && t4d) { // POE+
                currentPOEState = 2;
            } else if (atDet && !t3d && t4d) { // POE++ Type 3
                currentPOEState = 3;
            } else if (atDet && t3d && !t4d) { // POE++ Type 4
                currentPOEState = 4;
            } else {
                currentPOEState = 5;
            }
        }

        if (!t3d && !t4d) { // 30W POE Module
            LOG.info("30W POE module detected");
        } else { // 85W POE Module
            LOG.info("85W POE module detected");
        }

        LOG.debug(String("IO states: " + String(ioStates, BIN) + ", atDet: " + String(atDet) + ", t3d: " + String(t3d) + ", t4d: " + String(t4d)));

        // Set brightness limit dependent on POE type
        executePoeDependencies(currentPOEState);
    } else {
        LOG.error("[CL-SN-03] IO Multiplexer Unresponsive");
    }

    timeStamp = millis();
}

/**************************************************************************/
/**
 * @brief Executes the necessary actions based on the detected PoE type.
 *
 * @param poeType The detected PoE type (1 = POE, 2 = POE+, 3 = POE++ Type 3, 4 = POE++ Type 4, 5 = No PoE).
 */
/**************************************************************************/
void PowerOverEthernet::executePoeDependencies(uint8_t poeType) {

    switch (poeType) {
    case 1: // POE
        LOG.info(String("POE Type: POE"));
#ifdef AD_CL_CL1
        _matrixController->setLEDBrightnessLimit(MAX_BRIGHTNESS);
#elif defined(AD_CL_CL1S)
        _matrixController->setLEDBrightnessLimit(MAX_BRIGHTNESS);
#elif defined(AD_CL_CL1_PLUS)
        _matrixController->setLEDBrightnessLimit(MAX_BRIGHTNESS);
#elif defined(AD_CL_CL16)
        _powerSensor->setThreshold(12.5);
#else
        _matrixController->setLEDBrightnessLimit(LOW_ENERGY_BRIGHTNESS);
        LOG.error("[CL-PW-01] Insufficient PoE Power Supply");
#endif
        break;
    case 2: // POE+
        LOG.info(String("POE Type: POE+"));
#ifdef AD_CL_CL1
        _matrixController->setLEDBrightnessLimit(MAX_BRIGHTNESS);
#elif defined(AD_CL_CL1S)
        _matrixController->setLEDBrightnessLimit(MAX_BRIGHTNESS);
#elif defined(AD_CL_CL1_PLUS)
        _matrixController->setLEDBrightnessLimit(MAX_BRIGHTNESS);
#elif defined(AD_CL_CL2)
        _matrixController->setLEDBrightnessLimit(MAX_BRIGHTNESS);
#elif defined(AD_CL_CL16)
        _powerSensor->setThreshold(25.0);
#else
        _matrixController->setLEDBrightnessLimit(LOW_ENERGY_BRIGHTNESS);
        LOG.error("[CL-PW-01] Insufficient PoE Power Supply");
#endif
        break;
    case 3: // POE++ Type 3
        LOG.info(String("POE Type: POE++ Type 3"));
#ifdef AD_CL_CL16
        _powerSensor->setThreshold(50.5);
#else
        _matrixController->setLEDBrightnessLimit(MAX_BRIGHTNESS);
#endif
        break;
    case 4: // POE++ Type 4

        LOG.info(String("POE Type: POE++ Type 4"));
#ifdef AD_CL_CL16
        _powerSensor->setThreshold(71.0);
#else
        _matrixController->setLEDBrightnessLimit(MAX_BRIGHTNESS);
#endif
        break;
    case 5: // No POE
        LOG.warning(String("Unknown POE Type"));
#ifdef AD_CL_CL16
        _powerSensor->setThreshold(12.5);
#else
        _matrixController->setLEDBrightnessLimit(LOW_ENERGY_BRIGHTNESS);
#endif
        break;
    default:
        break;
    }
}
