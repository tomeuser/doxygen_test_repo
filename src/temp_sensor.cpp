/**************************************************************************/
/**
 * @file temp_sensor.cpp
 * @author Tom Meuser (t.meuser@innovaze-media.com)
 * @brief This file defines all methods of class TempSensor which were declared
 * in temp_sensor.h
 * @version 0.1
 * @date 2024-09-19
 *
 * @copyright Copyright (c) 2024, Adunas GmbH
 *            All rights reserved.
 *            Unauthorized copying of this file, via any medium is strictly
 *            prohibited Proprietary and confidential *
 */
/**************************************************************************/
#include "temp_sensor.h"

#ifndef AD_CL_CL16
/**************************************************************************/
/**
 * @brief Construct a new TempSensor object.
 *
 * @param wire Pointer to the TwoWire interface (I2C bus)
 * @param mqttClient mqttClient Pointer to the MqttClient for sending data via MQTT.
 * @param matrixController matrixController Pointer to the MatrixController object for controlling the LED matrix
 */
/**************************************************************************/
TempSensor::TempSensor(TwoWire *wire, MqttClient *mqttClient, MatrixController<CL_CHAMBER_COUNT> *matrixController) : Adafruit_BMP3XX() {
    _mqttClient = mqttClient;
    _matrixController = matrixController;
    _wire = wire;
}
#endif

/**************************************************************************/
/**
 * @brief Construct a new TempSensor object.
 *
 * @param wire Pointer to the TwoWire interface (I2C bus)
 * @param mqttClient mqttClient Pointer to the MqttClient for sending data via MQTT.
 */
/**************************************************************************/
TempSensor::TempSensor(TwoWire *wire, MqttClient *mqttClient) {
    _mqttClient = mqttClient;
    _wire = wire;
}

/**************************************************************************/
/**
 * @brief Initializes the temperature sensor and sets up its configuration
 *
 * @param mqttTopic The MQTT topic to which temperature data will be published.
 * @param interval The interval in milliseconds between temperature readings.
 */
/**************************************************************************/
void TempSensor::begin(const char *mqttTopic, unsigned long interval) {
    _mqttTopic = mqttTopic;
    _interval = interval;
    LOG.info("Check for temperature sensor...");
    for (int i = 0; i < MAX_CON_TRIES; i++) {
        if (!Adafruit_BMP3XX ::begin_I2C(I2C_BME_ADDRESS, _wire)) {
            _sensorActive = false;
        } else {
            LOG.info("Found valid temperature sensor");
            _sensorActive = true;
            // Set up oversampling and filter initialization
            Adafruit_BMP3XX ::setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
            Adafruit_BMP3XX ::setPressureOversampling(BMP3_OVERSAMPLING_4X);
            Adafruit_BMP3XX ::setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
            Adafruit_BMP3XX ::setOutputDataRate(BMP3_ODR_50_HZ);
            _temperatureTimeStamp = millis();
            return;
        }
    }
    if (!_sensorActive) {
        LOG.error("[CL-SN-01] Temperature Sensor Unresponsive");
    }
}

/**************************************************************************/
/**
 * @brief Periodically checks the temperature and takes action in case of overheating
 */
/**************************************************************************/
void TempSensor::loop() {
    if (_sensorActive) {
        if (millis() - _temperatureTimeStamp > _interval) {
            if (!Adafruit_BMP3XX::performReading()) {
                LOG.error("[CL-SN-01] Temperature Sensor Unresponsive");
                _sensorActive = false;
                return;
            } else {
                double temperature = Adafruit_BMP3XX::temperature;
                if (temperature != currentTemperature) {
                    if (_mqttClient->connected()) {
                        _mqttClient->publish(_mqttTopic.c_str(), String(temperature).c_str());
                        currentTemperature = temperature;
                    }
                }
                if (overheated) {
                    if (temperature <= OVERHEAT_STOP) {
                        // Enable matrix
                        overheated = false;
                        publishedPreWarning = false;
#ifndef AD_CL_CL16
                        _matrixController->enableMatrixControl(true);
#endif
                        LOG.info("Device cooled down to operating temperature");
                    }
                } else {
                    if (temperature >= OVERHEAT_START) {
                        // Disable matrix
                        LOG.error("[CL-OV-01] Device Overheated");
#ifndef AD_CL_CL16
                        _matrixController->enableMatrixControl(false);
#endif
                        overheated = true;
                    } else if (temperature > OVERHEAT_STOP) {
                        if (!publishedPreWarning) {
                            LOG.warning("[CL-OV-02] High Temperature Warning");
                            publishedPreWarning = true;
                        }
                    }
                }
            }
            _temperatureTimeStamp = millis();
        }
    } else {
    }
}
