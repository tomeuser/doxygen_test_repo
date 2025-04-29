/**************************************************************************/
/**
 * @file memory_manager.h
 * @author Tom Meuser (t.meuser@innovaze-media.com)
 * @brief This is the headerfile of the class MemoryManager declaring all
 * varibales and methods used by this class
 * @version 0.1
 * @date 2024-04-25
 *
 * @copyright Copyright (c) 2024, Adunas GmbH
 *            All rights reserved.
 *            Unauthorized copying of this file, via any medium is strictly
 *            prohibited Proprietary and confidential *
 */
/**************************************************************************/
#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H
#include "../../../src/const.h"
#include <ETH.h>
#include <Preferences.h>

#include "ArduinoJson.h"
#include "Client.h"
#include "HttpClient/HttpClient.h"
#include "logger.h"
#include <mqtt_client.h>

// SSL Certificate

class MemoryManager {
private:
    Preferences _preferences;
    ETHClass *_ethernetInterface;
    MqttClient *_mqttClient;

    String _macAddress;

    // Network
    bool _dhcp;
    IPAddress _localIp;
    IPAddress _subnetMask;
    IPAddress _gateway;
    IPAddress _dns;

    // Ota
    String _otaVersionFile;

    void _readMemory();

public:
    MemoryManager(ETHClass *ethernetInterface, MqttClient *mqttClient) {
        _ethernetInterface = ethernetInterface;
        _mqttClient = mqttClient;
    }

    void begin();
    void reset();
    void updateMemory(DynamicJsonDocument &doc);

    // Getter
    String getMacAddress();
    bool enableHealthData();
    bool enableDebug();

    // Network
    bool getDhcp();
    IPAddress getLocalIp();
    IPAddress getSubnetMask();
    IPAddress getGateway();
    IPAddress getDns();

    String getOtaVersionFile();

    // Utility
    IPAddress ipFromString(const char *ip);
    void printAll();
    const char *readMacAdress();
};

#endif