/**************************************************************************/
/**
 * @file light_sensor.hpp
 * @author Tom Meuser (t.meuser@innovaze-media.com)
 * @brief This file defines all methods of the template class LightSensor which were declared
 * in light_sensor.h
 * @version 0.1
 * @date 2024-09-19
 *
 * @copyright Copyright (c) 2024, Adunas GmbH
 *            All rights reserved.
 *            Unauthorized copying of this file, via any medium is strictly
 *            prohibited Proprietary and confidential *
 */
/**************************************************************************/
#include "light_sensor.h"

/**************************************************************************/
/**
 * @brief Constructor to initialize a new LightSensor object.
 *
 * Initializes internal variables and configures the I2C addresses for each sensor.
 *
 * @tparam  The number of LED matrixes that contain a light sensor.
 * @param mqttClient A pointer to the MQTT client for publishing battery level.
 * @param wire A pointer to the I2C interface.
 */
/**************************************************************************/
template <int ledMatrixCount>
inline LightSensor<ledMatrixCount>::LightSensor(MqttClient *mqttClient, TwoWire *wire) {
    _wire = wire;
    _mqttClient = mqttClient;
    for (int i = 0; i < ledMatrixCount; ++i) {
        ads[i] = Adafruit_ADS1015();
        adsInitialized[i] = false;
        isReading[i] = false;
        latestBatteryVoltage[i] = 0;
        latestLightSensorValues[i] = 0;
        adsChannelReadout[i] = 0;
        compareStates[i] = 0;
        compareCounter[i] = 0;
        oldLightSensorValues[i] = 0;

        if (i == 0) {
            i2cAddresses[i] = LS_I2C_ADDRESS_CL1;
        } else if (i == 1) {
            i2cAddresses[i] = LS_I2C_ADDRESS_CL2;
        } else if (i == 2) {
            i2cAddresses[i] = LS_I2C_ADDRESS_CL3;
        } else if (i == 3) {
            i2cAddresses[i] = LS_I2C_ADDRESS_CL4;
        }
    }
}

/**************************************************************************/
/**
 * @brief Tries to initialize the ADS1015 sensors within a given timeout
 *        period and starts reading sensor data.
 *
 * @tparam The number of LED matrixes to support.
 * @param batteryVoltageTopic MQTT topic to publish battery voltage data.
 */
/**************************************************************************/
template <int ledMatrixCount>
inline void LightSensor<ledMatrixCount>::begin(const char *batteryVoltageTopic) {
    _batteryTopic = batteryVoltageTopic;
    LOG.info("Check for light sensor...");

    // Initialize each ADS1015 sensor
    bool allInitializedComplete = false;
    unsigned long startTime = millis();
    while (!allInitializedComplete && millis() - startTime < LS_INIT_TIMEOUT) { // check if all sensors are initialized or if the timeout is reached
        allInitializedComplete = true;
        for (int i = 0; i < ledMatrixCount; i++) {
            if (!adsInitialized[i]) {
                ads[i].setGain(LS_ADC_GAIN);
                if (ads[i].begin(i2cAddresses[i], &Wire)) {
                    adsInitialized[i] = true;
                } else {
                    allInitializedComplete = false;
                }
                delay(10);
            }
        }
    }

    for (int i = 0; i < ledMatrixCount; i++) {
        if (adsInitialized[i]) {
            LOG.info(String("Chamber " + String(i + 1) + ": Found valid light sensor"));
        } else {
            LOG.error(String("[CL-SN-02] Chamber " + String(i + 1) + ": Light Sensor Unresponsive"));
        }
    }

    // Start asynchronous reading
    for (int i = 0; i < ledMatrixCount; i++) {
        startReadLightSensor(i);
    }
}

/**************************************************************************/
/**
 * @brief Maps analog-to-digital converter values to voltage values.
 *
 * Converts the ADC value to a corresponding voltage based on input and output ranges.
 *
 * @tparam  The number of LED matrixes to support.
 * @param x ADC value to be converted.
 * @param in_min Minimum value of the input range.
 * @param in_max Maximum value of the input range.
 * @param out_min Minimum value of the output range.
 * @param out_max Maximum value of the output range.
 * @return The corresponding voltage value.
 */
/**************************************************************************/
template <int ledMatrixCount>
inline int LightSensor<ledMatrixCount>::mapAdToVoltageValues(int x, int in_min, int in_max, int out_min, int out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/**************************************************************************/
/**
 * @brief Main loop function for the light sensor.
 *
 * Periodically reads light sensor and battery level values.
 *
 * @tparam The number of LED matrixes to support.
 */
/**************************************************************************/
template <int ledMatrixCount>
inline void LightSensor<ledMatrixCount>::loop() {
    if (millis() - readTimeStamp > readInterval) {
        for (int i = 0; i < ledMatrixCount; i++) {
            if (adsInitialized[i]) {
                if (isReading[i]) {
                    if (isReadComplete(i)) {
                        // Alternates between reading battery level (voltage) and light sensor of each matrix
                        if (adsChannelReadout[i] == 1) {
                            processBatteryData(i);
                            startReadLightSensor(i);
                        } else if (adsChannelReadout[i] == 0) {
                            processLightSensorData(i);
                            startReadBatteryVoltage(i);
                        }
                    }
                }
            }
        }
        readTimeStamp = millis();
    }
    if (millis() - publishTimeStamp > publishInterval) {
#ifdef AD_CL_CLX // Only publish battery level for CLX Series
        if (latestBatteryVoltage[0] != oldBatteryVoltage) {
            oldBatteryVoltage = latestBatteryVoltage[0];
            // Battery level
            // 9.6V = 0%
            // 12.4V = 100%
            int percent = (int)(((latestBatteryVoltage[0] - 9.6f) / 2.8f) * 100.0f);
            if (percent > 100) {
                percent = 100;
            }
            _mqttClient->publish(String(_batteryTopic).c_str(), String(percent).c_str());
        }
#endif
        publishTimeStamp = millis();
    }
}

/**************************************************************************/
/**
 * @brief Starts reading the light sensor value for the given sensor.
 *
 * @tparam The number of LED matrixes to support.
 * @param sensor Index of the sensor to start reading.
 */
/**************************************************************************/
template <int ledMatrixCount>
inline void LightSensor<ledMatrixCount>::startReadLightSensor(int sensor) {
    if (adsInitialized[sensor]) {
        adsChannelReadout[sensor] = 0;                                       // Mode reade light sensor
        ads[sensor].startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_0, false); // Light Sensor
        isReading[sensor] = true;
    }
}

/**************************************************************************/
/**
 * @brief Starts reading the battery voltage value for the given sensor.
 *
 * @tparam  The number of LED matrixes to support.
 * @param sensor Index of the sensor to start reading.
 */
/**************************************************************************/
template <int ledMatrixCount>
inline void LightSensor<ledMatrixCount>::startReadBatteryVoltage(int sensor) {
    if (adsInitialized[sensor]) {
        adsChannelReadout[sensor] = 1;                                       // Mode read battery level (voltage)
        ads[sensor].startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_1, false); // Battery
        isReading[sensor] = true;
    }
}

/**************************************************************************/
/**
 * @brief Checks if the reading from the given sensor is complete.
 *
 * @tparam  The number of LED matrixes to support.
 * @param sensor Index of the sensor to check.
 * @return true if the reading is complete, false otherwise.
 */
/**************************************************************************/
template <int ledMatrixCount>
inline bool LightSensor<ledMatrixCount>::isReadComplete(int sensor) {
    if (adsInitialized[sensor] && ads[sensor].conversionComplete()) {
        return true;
    } else {
        return false;
    }
}

/**************************************************************************/
/**
 * @brief Processes the most recent battery voltage data from the given sensor.
 *
 * @tparam The number of LED matrixes to support.
 * @param sensor Index of the sensor to process the battery voltage data from.
 */
/**************************************************************************/
template <int ledMatrixCount>
inline void LightSensor<ledMatrixCount>::processBatteryData(int sensor) {
    if (adsInitialized[sensor]) {
        if (adsChannelReadout[sensor] == 1) {
            if (isReading[sensor]) {
                int16_t adcValue = ads[sensor].getLastConversionResults();
                float voltage = 0.0f;
                // Convert AD-Converter values into voltage values
                voltage = float(mapAdToVoltageValues(adcValue, MIN_AD_16BIT_VALUE, MAX_AD_16BIT_VALUE, MIN_VOLTAGE, MAX_VOLTAGE)) / 100.0f; //
                latestBatteryVoltage[sensor] = voltage;
                isReading[sensor] = false;
            } else {
                LOG.debug("No new value was requested, can't read new value");
            }
        } else {
            LOG.debug("Cant't read battery data because light sensor was read");
        }
    } else {
        LOG.debug("N/A (N/A V)\t");
    }
}

/**************************************************************************/
/**
 * @brief Processes the most recent light sensor data from the given sensor.
 *
 * @tparam The number of LED matrixes to support.
 * @param sensor Index of the sensor to process the light sensor data from.
 */
/**************************************************************************/
template <int ledMatrixCount>
inline void LightSensor<ledMatrixCount>::processLightSensorData(int sensor) {
    if (adsInitialized[sensor]) {
        if (adsChannelReadout[sensor] == 0) {
            if (isReading[sensor]) {
                int16_t adcValue = ads[sensor].getLastConversionResults();
                float voltage = adcValue * multiplier;
                // Convert AD-Converter values into lux values
                int lux = adValuesToLux(adcValue);
                // LOG.debug(String("Chamber " + String(sensor + 1) + ": " + String(lux)));

                // Check if any cue powered on or powered off. In that case method checkState()
                // compares old and new light sensor values to check if cue actually powered on or powered off
                CheckState(sensor, lux);

                latestLightSensorValues[sensor] = lux;
                isReading[sensor] = false;
            } else {
                LOG.debug("No new value was requested, can't read new value");
            }

        } else {
            LOG.debug("Cant't read light sensor data because battery voltage was read");
        }

    } else {
        LOG.debug("N/A (N/A V)\t");
    }
}

/**************************************************************************/
/**
 * @brief Converts the ADC value to a corresponding lux value.
 *
 * @tparam  The number of LED matrixes to support.
 * @param adcValue ADC value from the sensor.
 * @return The lux value.
 */
/**************************************************************************/
template <int ledMatrixCount>
inline int LightSensor<ledMatrixCount>::adValuesToLux(int16_t adcValue) {
    // Calculation through linear regression https://www.allaboutcircuits.com/projects/design-a-luxmeter-using-a-light-dependent-resistor/
    return (pow(adcValue, 1.0573) * pow(10, -0.3424)); // our calculation
}

/**************************************************************************/
/**
 * @brief Compares previous and latest light sensor data after turn on and
 *        turn off command got executed
 *
 * @tparam The number of LED matrixes to support.
 * @param sensor Index of the sensor to check.
 * @param newState The new sensor value.
 */
/**************************************************************************/
template <int ledMatrixCount>
inline void LightSensor<ledMatrixCount>::CheckState(int sensor, int newState) {
    if (adsInitialized[sensor]) {
        if (compareStates[sensor] == 0) {
            return;
        } else if (compareStates[sensor] == 1) { // Turned on
            if (newState > latestLightSensorValues[sensor] + LS_SWITCH_STATE_INDICATOR) {
                compareStates[sensor] = 0;
                compareCounter[sensor] = 0;
                return;
            } else {
                if (compareCounter[sensor] < 1) {
                    compareCounter[sensor]++;
                } else {
                    LOG.debug("Previous Brightness:" + String(latestLightSensorValues[sensor]));
                    LOG.debug("Current Brightness:" + String(newState));
                    LOG.warning(String("[CL-SN-04] Chamber " + String(sensor + 1) + " fails to Turn On"));
                    compareStates[sensor] = 0;
                    compareCounter[sensor] = 0;
                }
            }
        } else if (compareStates[sensor] == 2) { // Turned off
            if (newState < latestLightSensorValues[sensor] - LS_SWITCH_STATE_INDICATOR) {
                compareStates[sensor] = 0;
                compareCounter[sensor] = 0;
                return;
            } else {
                if (compareCounter[sensor] < 1) {
                    compareCounter[sensor]++;
                } else {
                    LOG.debug("Previous Brightness:" + String(latestLightSensorValues[sensor]));
                    LOG.debug("Current Brightness:" + String(newState));
                    LOG.warning(String("[CL-SN-05] Chamber " + String(sensor + 1) + " fails to Turn Off"));
                    compareStates[sensor] = 0;
                    compareCounter[sensor] = 0;
                }
            }
        }
    }
}

/**************************************************************************/
/**
 * @brief Triggers the process to check weather chamber got actually powered off .
 *
 * @tparam The number of LED matrixes to support.
 * @param sensor Index of the sensor whose values should be used to check weather
 *               chamber got actually powered off.
 */
/**************************************************************************/
template <int ledMatrixCount>
inline void LightSensor<ledMatrixCount>::checkPoweredOff(int sensor) {
    compareCounter[sensor] = 0;
    compareStates[sensor] = 2;
    readTimeStamp = millis();
}

/**************************************************************************/
/**
 * @brief Triggers the process to check weather chamber got actually powered on .
 *
 * @tparam The number of LED matrixes to support.
 * @param sensor Index of the sensor whose values should be used to check weather
 *               chamber got actually powered off.
 */
/**************************************************************************/
template <int ledMatrixCount>
inline void LightSensor<ledMatrixCount>::checkPoweredOn(int sensor) {
    compareCounter[sensor] = 0;
    compareStates[sensor] = 1;
    readTimeStamp = millis();
}
