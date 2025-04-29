/**************************************************************************/
/**
 * @file main.cpp
 * @author Tom Meuser (t.meuser@innovaze-media.com)
 * @brief This file is the main file of CL Series
 * @version 1.0.3
 * @date 2024-11-28
 *
 * @copyright Copyright (c) 2024, Adunas GmbH
 *            All rights reserved.
 *            Unauthorized copying of this file, via any medium is strictly prohibited
 *            Proprietary and confidential
 */
/**************************************************************************/

#include "config.h" // import libraries and class instances

void callback(char *topic, byte *payload, unsigned int length);

void setup() {
    // Check firmware version
    // TODO: Move this to the buildinfo Class
    String fwVersion;
    // add git hash to the firmware, if it is not a main release
    if (buildInfo::gitBranch == "main") {
        fwVersion = String(buildInfo::firmwareVersion);
    } else {
        fwVersion = String(String(buildInfo::firmwareVersion) + "-" + String(buildInfo::gitLastCommitHash));
    }
    // add a underscore at the end if local build, to signal potential changes
    if (buildInfo::buildEnvironment != "cloud") {
        fwVersion += "_";
    }

    // Setup serial interface
    Serial.begin(115200);

    // Setup ethernet interface
    ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_TYPE, ETH_CLK_MODE);
    ETH.setHostname(PRODUCT_TYPE);
    // Setup permanent storage
    memoryManager.begin();
    if (!memoryManager.getDhcp()) // Check if static ip got configured through mqtt config topic
        ETH.config(memoryManager.getLocalIp(), memoryManager.getSubnetMask(), memoryManager.getGateway(), memoryManager.getDns());

    // Set MQTT topic for Logging
    LOG.begin(&mqttClient, String(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_INFO)));
    LOG.info("Attempting network connection...");

    // Apply mqtt permanent settings
    mqttClient.setMQTTServerDomain(MQTT_HOST_NAME);
    mqttClient.setMQTTServerIP(MQTT_HOST_IP); // only as a backup, if the MQTT_HOST_NAME is empty
    mqttClient.setPort(MQTT_SERVER_PORT);

    // Subscribe topics
    mqttClient.subscribeOnConnect(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_CONFIG) + "/#");
    mqttClient.subscribeOnConnect(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_OTA_CHECK));
    mqttClient.subscribeOnConnect(String(PRODUCT_TYPE) + "/" + String(MQTT_OTA_CHECK));
    mqttClient.subscribeOnConnect(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_OTA_START));
    mqttClient.subscribeOnConnect(String(PRODUCT_TYPE) + "/" + String(MQTT_OTA_START));
    mqttClient.subscribeOnConnect(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_CONTROL) + "/#");

    // Publish to MQTT server after connected
    mqttClient.publishOnConnect(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_INFO) + "/product_type", String(PRODUCT_TYPE));
    mqttClient.publishOnConnect(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_INFO) + "/firmware_ver", fwVersion);
    mqttClient.publishOnConnect(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_INFO) + "/interface_ver", String(buildInfo::interfaceVersion));
    mqttClient.publishOnConnect(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_INFO) + "/mac_address", memoryManager.getMacAddress());
    mqttClient.publishNetworkDataOnConnect(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_INFO) + "/network");

    // Setup default messages
    mqttClient.setLastWillMessage(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_STATE), "offline");

    // Other mqtt settings
    mqttClient.setCallback(callback);
    mqttClient.setKeepAlive(MQTT_PING_INTERVAL);
    mqttClient.setBuffer(MQTT_MAX_PACKET_SIZE);

    // Ota settings
    otaClient.setInsecure(); // To prevent connection issues due to ssl certificate expiration
    otaUpdate.setUpdateAvailableTopic(String(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_OTA_AVAILABLE)).c_str());
    otaUpdate.setFwVersionFileUrl(memoryManager.getOtaVersionFile().c_str());

    // Setup I2C interface
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ);

    // Setup temperature sensor
    tempSensor.begin(String(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_INFO) + "/temperature").c_str(), TEMP_READ_INTERVAL);

    // On board io multiplexer
    onBoardMultiplexer.attach(Wire, I2C_ON_BOARD_MULTIPLEXER_ADDR);
    onBoardMultiplexer.polarity(PCA95x5::Polarity::ORIGINAL_ALL);
    onBoardMultiplexer.direction(PCA95x5::Direction::IN_ALL);
    // Check for connectivity
    LOG.info("Check for on board IO Multiplexer...");
    onBoardMultiplexer.read();
    if (onBoardMultiplexer.i2c_error() != 0) {
        LOG.error("[CL-SN-03] IO Multiplexer Unresponsive");
    } else {
        LOG.info("Found valid IO Multiplexer");
    }

    // Check poe type
    poe.init();

#ifndef AD_CL_CL16
    // Setup light sensor and battery monitoring
    lightSensor.begin(String(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_STATE) + "/battery_level").c_str());

#ifdef ACK_BUTTON // CueLight acknowledgment button
    // Setup led matrix
    matrixController.begin(String(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_STATE) + "/btn_1").c_str());
#else
    matrixController.begin();
#endif
#else
    // Power Sensor
    powerSensor.begin(String(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_INFO) + "/power").c_str());
    matrixController.begin(String(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_STATE)).c_str());
    statusLED.begin();

    // Turn on status led in green
    statusLED.configureColor(0, 255, 0);
    statusLED.turnOn();
#endif
}

void loop() {
    mqttClient.loop();
    matrixController.loop();
    tempSensor.loop();
#ifndef AD_CL_CL16
    lightSensor.loop();
#else
    powerSensor.loop();
    statusLED.loop();
#endif

    if (Serial.available()) {
        String consolMessage = "";
        while (Serial.available()) {
            // get the new byte:
            consolMessage += (char)Serial.read();
        }
        if (consolMessage.length() > 3 && consolMessage.length() < 10) {
            if (consolMessage.startsWith("AT+LOG=")) {
                int logLevel = consolMessage.substring(7, 8).toInt();
                if (logLevel >= 0 && logLevel <= 5) {
                    LOG.setLogLevel(LogLevel(logLevel));
                } else {
                    LOG.warning("Wrong input to change logging level");
                }
            }
        }
    }
}

/**************************************************************************/
/**
 * @brief This function is the MQTT callback function, which gets executed
 *          in case a MQTT Message from a subscribed topic arrives
 *
 * @param topic MQTT Topic under which the message got published
 * @param payload  MQTT Message
 * @param length Message length (Amount of characters)
 */
/**************************************************************************/
void callback(char *topic, byte *payload, unsigned int length) {
    String _topic = topic; // Important if you want to publish messages while executing callback

    // byte to const char* conversion
    char strPayload[length + 1]; // +1 for the \0 at the end of a string
    strncpy(strPayload, reinterpret_cast<char *>(payload), length);
    strPayload[length] = '\0';

    LOG.info(String("Incoming message [" + _topic + "]: " + strPayload));

    // Iterate through chamber until right chamber is picked
    for (int i = 0; i < CL_CHAMBER_COUNT; i++) {
        String clControlTopic = String(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_CONTROL) + "/cl_" + String(i + 1)).c_str();
        String clColorTopic = String(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_CONTROL) + "/cl_" + String(i + 1) + "/color").c_str();
        String clBlinkTopic = String(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_CONTROL) + "/cl_" + String(i + 1) + "/blink").c_str();

        if (_topic == clControlTopic) {
            matrixController.controlMatrix(i, strPayload);
            return;
        } else if (_topic == clBlinkTopic) {
            matrixController.configureBlink(i, strPayload);
            return;
        } else if (_topic == clColorTopic) {
#ifndef AD_CL_CL16
            matrixController.configureColor(i, strPayload);
            return;
#endif
        }
    }

#ifdef AD_CL_CL16
    // StatusLED
    String clControlTopic = String(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_CONTROL) + "/cl_17").c_str();
    String clColorTopic = String(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_CONTROL) + "/cl_17" + "/color").c_str();
    String clBlinkTopic = String(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_CONTROL) + "/cl_17" + "/blink").c_str();
    String msg = String(strPayload);
    if (_topic == clControlTopic) {
        if (msg == "on") {
            statusLED.turnOn();
        } else if (msg == "off") {
            statusLED.turnOff();
        }
        return;
    } else if (_topic == clBlinkTopic) {
        if (msg == "1" || msg == "true") { // Start blink
            statusLED.startBlinking();
        } else if (msg == "0" || msg == "false") { // Start blink
            statusLED.stopBlinking();
        }
        return;
    } else if (_topic == clColorTopic) {
        statusLED.configureColor(strPayload);
        return;
    }
#endif

    // Config
    if (_topic == String(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_CONFIG))) {
        // Allocate JSON document and deserialize MQTT message
        DynamicJsonDocument incomingMessage(500);
        DeserializationError error = deserializeJson(incomingMessage, strPayload);
        // Checks if payload of the MQTT-message is JSON formatted
        if (!error) {
            memoryManager.updateMemory(incomingMessage);
            otaUpdate.checkJsonOta(incomingMessage);
            return;
        }
    } else if (_topic == String(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_OTA_CHECK))) {
        otaUpdate.startOfficialVersionRequest();
    } else if (_topic == String(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_OTA_START))) {
        otaUpdate.startOfficialUpdate();
    } else if (_topic == String(String(PRODUCT_TYPE) + "/" + String(MQTT_OTA_CHECK))) {
        otaUpdate.startOfficialVersionRequest();
    } else if (_topic == String(String(PRODUCT_TYPE) + "/" + String(MQTT_OTA_START))) {
        otaUpdate.startOfficialUpdate();
    }

    // Set logging level
    if (_topic == String(String(PRODUCT_TYPE) + "/" + memoryManager.getMacAddress() + "/" + String(MQTT_CONFIG) + "/logging")) {
        uint8_t level = String(strPayload).toInt(); // 0 = NONE, 1 = ERROR, 2 = WARNING, 3 = INFO, 4 = DEBUG
        if (level <= 4 && level >= 0) {
            LOG.setLogLevel(LogLevel(level));
        } else {
            LOG.warning("[CL-CM-01] Invalid Command Format");
        }
    }
}
