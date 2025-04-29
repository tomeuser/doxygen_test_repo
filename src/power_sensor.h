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
#ifndef POWER_SENSOR_H
#define POWER_SENSOR_H

#include "logger.h"
#include <Wire.h>
#include <mqtt_client.h>

#include "Adafruit_INA219.h"

#ifndef I2C_EXTERNAL_POWER_SENSOR_ADDR_1
#define I2C_EXTERNAL_POWER_SENSOR_ADDR_1 (0x40)
#endif
#ifndef I2C_EXTERNAL_POWER_SENSOR_ADDR_2
#define I2C_EXTERNAL_POWER_SENSOR_ADDR_2 (0x41)
#endif
#ifndef BASE_PCB_POWER_CONSUMPTION
#define BASE_PCB_POWER_CONSUMPTION 1 // Avarage Power consumption of the components on the base board (MCU + ETH). This needs to be added to the power measurement because the Shunt is located after them.
#endif

/*
    Address Matrix:
    GND 	GND 	0x40
    GND 		    0x41
    GND 	SDA 	0x42
    GND 	SCL 	0x43
    GND 		    0x44
                    0x45
    SDA 		    0x46
    SCL 		    0x47
    SDA 	GND 	0x48
    SDA 		    0x49
    SDA 	SDA 	0x4A
    SDA 	SCL 	0x4B
    SCL 	GND 	0x4C
    SCL 		    0x4D
    SCL 	SDA 	0x4E
    SCL 	SCL 	0x4F
*/

class PowerSensor {
public:
    PowerSensor(TwoWire *wire, MqttClient *mqttClient);
    Adafruit_INA219 _ina219_1;
    Adafruit_INA219 _ina219_2;
    void begin(const char *mqttTopic);
    void loop();
    void setThreshold(float value);
    bool powerUnderThreshold();
    bool notified80 = false;
    bool notified90 = false;
    bool notifiedExceed = false;

private:
    MqttClient *_mqttClient;
    TwoWire *_wire;

    String _mqttTopic;
    unsigned long _interval = 1000;
    unsigned long _powerTimeStamp = 0;
    bool _sensorActive = false;
    unsigned long _mqttInterval = 5000;
    unsigned long _mqttTimeStamp = 0;

    float _threshold = 12.5;
    float lastPowerValue = 0.0;
};
#endif