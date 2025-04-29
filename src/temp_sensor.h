/**************************************************************************/
/**
 * @file temp_sensor.h
 * @author Tom Meuser (t.meuser@innovaze-media.com)
 * @brief This is the header file of class temp_sensor.h declaring all variables
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
#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include <Wire.h>
#include <mqtt_client.h>

#include "Adafruit_BMP3XX.h"
#include "logger.h"
#ifndef AD_CL_CL16
#include "matrix_controller.h"
#endif
#define SEALEVELPRESSURE_HPA (1018)
#define I2C_BME_ADDRESS 0x76
#define MAX_CON_TRIES 10

class TempSensor : private Adafruit_BMP3XX {
public:
#ifndef AD_CL_CL16
    TempSensor(TwoWire *wire, MqttClient *mqttClient, MatrixController<CL_CHAMBER_COUNT> *matrixController);
#endif
    TempSensor(TwoWire *wire, MqttClient *mqttClient);

    void begin(const char *mqttTopic, unsigned long interval);
    void loop();

private:
    MqttClient *_mqttClient;
#ifndef AD_CL_CL16
    MatrixController<CL_CHAMBER_COUNT> *_matrixController;
#endif

    String _mqttTopic;
    unsigned long _interval;
    unsigned long _temperatureTimeStamp = 0;
    bool _sensorActive = false;
    TwoWire *_wire;
    int skipFirstRead = 0;
    double lastTemperatureRead = 0;
    double currentTemperature = 0;
    bool overheated = false;
    bool publishedPreWarning = false;
};
#endif