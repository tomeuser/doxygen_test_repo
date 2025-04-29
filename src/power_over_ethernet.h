/**************************************************************************/
/**
 * @file power_over_ethernet.h
 * @author Tom Meuser (t.meuser@innovaze-media.com)
 * @brief This is the header file of class power_over_ethernet.h declaring all variables
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
#ifndef POE_H
#define POE_H

#ifndef AD_CL_CL16
#include "matrix_controller.h"
#else
#include "power_sensor.h"
#endif
#include "logger.h"
#include "mqtt_client.h"
#include <PCA95x5.h>

class PowerOverEthernet {
public:
#ifndef AD_CL_CL16
    PowerOverEthernet(MqttClient *mqttClient, PCA9535 *ioMultiplexer, MatrixController<CL_CHAMBER_COUNT> *matrixController);
#else
    PowerOverEthernet(MqttClient *mqttClient, PCA9535 *ioMultiplexer, PowerSensor *powerSensor);
#endif
    void init();
    void executePoeDependencies(uint8_t poeType);

private:
    MqttClient *_mqttClient;
    PCA9535 *_ioMultiplexer;
#ifndef AD_CL_CL16
    MatrixController<CL_CHAMBER_COUNT> *_matrixController;
#else
    PowerSensor *_powerSensor;
#endif
    unsigned long timeStamp = 0;
    unsigned long interval = POE_READ_INTERVAL;
};

#endif