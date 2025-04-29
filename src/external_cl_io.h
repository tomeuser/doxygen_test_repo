/**************************************************************************/
/**
 * @file external_cl_io.h
 * @author Tom Meuser (t.meuser@innovaze-media.com)
 * @brief This is the header file of the template class ExternalClIO declaring all variables
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
#ifndef EXTERNAL_CL_IO
#define EXTERNAL_CL_IO

#include "power_sensor.h"
#include <ArduinoJson.h>
#include <PCA95x5.h>
#include <mqtt_client.h>

template <int chamberCount>
class ExternalClIO {
public:
    ExternalClIO(MqttClient *mqttClient, PCA9535 *externalMultiplexer_1, PCA9535 *externalMultiplexer_2, PowerSensor *powerSensor);
    void begin(const char *topic);
    void loop();
    void controlMatrix(int chamber, const char *incomingMessage);
    void configureBlink(int chamber, const char *incomingMessage);

private:
    MqttClient *_mqttClient;
    PowerSensor *_powerSensor;
    PCA9535 *_externalMultiplexer[EXT_MULTIPLEXER_COUNT]; // each io multiplexer contains 8 inputs and 8 outputs
    bool _foundMultiplexer[EXT_MULTIPLEXER_COUNT] = {false, false};

    // Inputs
    PCA95x5::Port::Port _input[EXT_MULTIPLEXER_IO_INPUTS] = {PCA95x5::Port::P07, PCA95x5::Port::P06, PCA95x5::Port::P05, PCA95x5::Port::P04, PCA95x5::Port::P13, PCA95x5::Port::P12, PCA95x5::Port::P11, PCA95x5::Port::P10};
    unsigned long inputInterval = READ_INTERVALE;
    unsigned long inputTimestamp = 0;
    unsigned int _integrator[chamberCount];
    unsigned int _previousInputState[chamberCount];
    String _topic;

    // Outputs
    PCA95x5::Port::Port _output[EXT_MULTIPLEXER_IO_OUTPUTS] = {PCA95x5::Port::P00, PCA95x5::Port::P01, PCA95x5::Port::P02, PCA95x5::Port::P03, PCA95x5::Port::P14, PCA95x5::Port::P15, PCA95x5::Port::P16, PCA95x5::Port::P17};
    bool _matrixState[chamberCount];
    bool _blinkState[chamberCount];
    bool _generalBlinkState = false;
    unsigned long previousMillisBlink = 0;
    void startBlink(int chamber);
    void stopBlink(int chamber);
    void debounce(bool input, unsigned int *integrator, unsigned int *output);
};
#include "external_cl_io.hpp"

#endif