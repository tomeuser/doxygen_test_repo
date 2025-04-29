/**************************************************************************/
/**
 * @file config.h
 * @author Tom Meuser (t.meuser@innovaze-media.com),
 *         Daniel Ahlers (d.ahlers@innovaze-media.com)
 * @brief This is the config file. It contains all class instances and imports external libraries
 * @version 1.0.3
 * @date 2024-11-28
 *
 * @copyright Copyright (c) 2024, Adunas GmbH
 *            All rights reserved.
 *            Unauthorized copying of this file, via any medium is strictly prohibited
 *            Proprietary and confidential
 */
/**************************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

/**************************************************************************/
// Import libraries
/**************************************************************************/
#include "const.h" // Include all macros

#include "buildinfo.h"

// Include logger
#include "logger.h"

// Include all libraries
#include "ArduinoJson.h"
#include "light_sensor.h"
#include "memory_manager.h"
#include "mqtt_client.h"
#include "ota_update.h"
#include "power_over_ethernet.h"
#include "status_led.h"
#include "temp_sensor.h"
#include <Arduino.h>
#include <ETH.h>
#include <PCA95x5.h>
#include <WiFiClientSecure.h>
#ifdef AD_CL_CL16
#include "external_cl_io.h"
#include "power_sensor.h"
#else
#include "matrix_controller.h"
#endif

/**************************************************************************/
// Class instances
/**************************************************************************/
// MQTT client
#ifdef SSL
MqttClient mqttClient(&ETH, true);
#else
MqttClient mqttClient(&ETH);
#endif

// Encrypted network client
WiFiClientSecure otaClient;
// OTA update
OtaUpdate otaUpdate(&ETH, &otaClient, &mqttClient);

// Permanent memory manager
MemoryManager memoryManager(&ETH, &mqttClient);

// On board multiplexer
PCA9535 onBoardMultiplexer;

#ifdef AD_CL_CL16
// External io multiplexer
PCA9535 externalMultiplexer_1;
PCA9535 externalMultiplexer_2;

// Power Sensor
PowerSensor powerSensor(&Wire, &mqttClient);

// LED matrix controller
ExternalClIO<CL_CHAMBER_COUNT> matrixController(&mqttClient, &externalMultiplexer_1, &externalMultiplexer_2, &powerSensor);
PowerOverEthernet poe(&mqttClient, &onBoardMultiplexer, &powerSensor);
StatusLED statusLED(1, LED_MATRIX_PIN, 255);

// Temperature sensor
TempSensor tempSensor(&Wire, &mqttClient);

#else
// Light Sensor
LightSensor<LED_MATRIX_COUNT> lightSensor(&mqttClient, &Wire);
#ifdef ACK_BUTTON
MatrixController<CL_CHAMBER_COUNT> matrixController(CL_CHAMBER_WIDTH, LED_MATRIX_HEIGHT, LED_MATRIX_PIN, &mqttClient, &lightSensor, &onBoardMultiplexer);
#else
MatrixController<CL_CHAMBER_COUNT> matrixController(CL_CHAMBER_WIDTH, LED_MATRIX_HEIGHT, LED_MATRIX_PIN, &mqttClient, &lightSensor);
#endif

// Temperature sensor
TempSensor tempSensor(&Wire, &mqttClient, &matrixController);
PowerOverEthernet poe(&mqttClient, &onBoardMultiplexer, &matrixController);
#endif

#endif // CONFIG_H