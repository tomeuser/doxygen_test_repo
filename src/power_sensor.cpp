#include "power_sensor.h"

PowerSensor::PowerSensor(TwoWire *wire, MqttClient *mqttClient) : _ina219_1(I2C_EXTERNAL_POWER_SENSOR_ADDR_1), _ina219_2(I2C_EXTERNAL_POWER_SENSOR_ADDR_2) {
    _mqttClient = mqttClient;
    _wire = wire;
}

void PowerSensor::begin(const char *mqttTopic) {
    _mqttTopic = mqttTopic;
    LOG.info("Check for power sensor...");

    if (_ina219_1.begin(_wire) && _ina219_2.begin(_wire)) {
        _sensorActive = true;
        LOG.info("Found valid power sensor");
        _ina219_1.setCalibration_150mohm();
        _ina219_2.setCalibration_150mohm();
        _powerTimeStamp = millis();
    } else {
        _sensorActive = false;
        LOG.error("[DC-SN-01] Power Sensor Unresponsive");
    }
}

void PowerSensor::loop() {
    if (_sensorActive) {
        if (millis() - _powerTimeStamp > _interval) {

            _wire->beginTransmission(I2C_EXTERNAL_POWER_SENSOR_ADDR_1);
            byte error1 = _wire->endTransmission();
            _wire->beginTransmission(I2C_EXTERNAL_POWER_SENSOR_ADDR_2);
            byte error2 = _wire->endTransmission();
            if (error1 != 0) {
                LOG.error("[DC-SN-01] Power Sensor 1 Unresponsive");
            } else if (error2 != 0) {
                LOG.error("[DC-SN-01] Power Sensor 2 Unresponsive");
            } else {

                float power = BASE_PCB_POWER_CONSUMPTION;

                power += (_ina219_1.getPower_mW() + _ina219_2.getPower_mW()) / 1000;

                if (power != lastPowerValue) {
                    lastPowerValue = power;
                    if (lastPowerValue < (_threshold * 0.8) && notifiedExceed) {
                        notifiedExceed = false;
                    }
                    if (lastPowerValue < (_threshold * 0.75) && notified90) {
                        notified90 = false;
                    }
                    if (lastPowerValue < (_threshold * 0.7) && notified80) {
                        notified80 = false;
                    }

                    if (lastPowerValue >= (_threshold * 0.8)) {
                        LOG.error("[DC-PW-03] Overloaded Power Consumption");
                        notifiedExceed = true;
                    } else if (lastPowerValue >= (_threshold * 0.75)) {
                        if (!notified90) {
                            LOG.warning("Power consumption achieved critical 90%");
                            notified90 = true;
                        }

                    } else if (lastPowerValue >= (_threshold * 0.7)) {
                        if (!notified80) {
                            LOG.info("Power consumption achieved 80%");
                            notified80 = true;
                        }
                    }

                    if (millis() - _mqttTimeStamp >= _mqttInterval) {
                        if (_mqttClient->connected()) {
                            _mqttClient->publish(_mqttTopic.c_str(), String(power, 2).c_str());
                        }
                        _mqttTimeStamp = millis();
                    }
                }
            }
            _powerTimeStamp = millis();
        }
    }
}

void PowerSensor::setThreshold(float value) {
    _threshold = value;
}

bool PowerSensor::powerUnderThreshold() {

    if (lastPowerValue >= (_threshold * 0.95)) {
        return false;
    } else {
        return true;
    }
}
