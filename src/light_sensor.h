/**************************************************************************/
/**
 * @file light_sensor.h
 * @author Tom Meuser (t.meuser@innovaze-media.com)
 * @brief This is the header file of the template class light_sensor.h declaring all variables
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

#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H
#include "logger.h"
#include <Adafruit_ADS1X15.h>
#include <Wire.h>
#include <mqtt_client.h>

#define MIN_VOLTAGE 800  // (:100)
#define MAX_VOLTAGE 1200 // (:100)
#define MIN_AD_16BIT_VALUE 470
#define MAX_AD_16BIT_VALUE 722

template <int ledMatrixCount>
class LightSensor {
public:
    LightSensor(MqttClient *mqttClient, TwoWire *wire);
    void begin(const char *batteryVoltageTopic);
    void loop();
    void checkPoweredOff(int sensor);
    void checkPoweredOn(int sensor);

private:
    Adafruit_ADS1015 ads[ledMatrixCount]; // Array of ADS1015 sensor instances (ads = Analog-Digital Sensor)
    TwoWire *_wire;
    MqttClient *_mqttClient;
    uint8_t i2cAddresses[ledMatrixCount];      // I2C addresses for each sensor
    bool adsInitialized[ledMatrixCount];       // Array to track initialization status
    uint8_t adsChannelReadout[ledMatrixCount]; // Define the channel that we are currently reading from
                                               // Channel 0 = Light sensor
                                               // Channel 1 = 12V Rail
    int latestLightSensorValues[ledMatrixCount];
    float latestBatteryVoltage[ledMatrixCount];

    int oldLightSensorValues[ledMatrixCount];
    float oldBatteryVoltage = 0.0f;

    uint8_t compareStates[ledMatrixCount];
    uint8_t compareCounter[ledMatrixCount];
    bool isReading[ledMatrixCount];
    String _batteryTopic;

    float _mult_value = 32017200.0, _pow_value = 1.5832;
    unsigned long _resistor = 10000; // 10kOhm

    // Gain setting for the ADS1015 (default is GAIN_TWOTHIRDS)
    const float multiplier = 2.0 / 1000; // 2mV per bit (for GAIN_ONE)
    unsigned long readTimeStamp = 0;
    unsigned long publishTimeStamp = 0;

    unsigned int readInterval = LS_READ_INTERVAL;
    unsigned long publishInterval = LS_READ_INTERVAL * 10;

    int mapAdToVoltageValues(int x, int in_min, int in_max, int out_min, int out_max);
    void startReadLightSensor(int sensor);
    void startReadBatteryVoltage(int sensor);
    bool isReadComplete(int sensor);
    void processLightSensorData(int sensor);
    void processBatteryData(int sensor);
    int adValuesToLux(int16_t adcValue);
    void CheckState(int sensor, int newState);
};
#include "light_sensor.hpp"
#endif // LIGHT_SENSOR_H
