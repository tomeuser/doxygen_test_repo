/**************************************************************************/
/**
 * @file mqtt_client.h
 * @author Tom Meuser (t.meuser@innovaze-media.com)
 * @brief This is the headerfile of the class MqttClient declaring all varibales
 * and methods used by this class
 * @version 0.1
 * @date 2024-03-27
 *
 * @copyright Copyright (c) 2024, Adunas GmbH
 *            All rights reserved.
 *            Unauthorized copying of this file, via any medium is strictly
 *            prohibited Proprietary and confidential *
 */
/**************************************************************************/
#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include "../../../src/const.h"
#include <ETH.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <vector>

#ifndef MQTT_SERVER_NETWORK_DEVICE_NUMBER
#define MQTT_SERVER_NETWORK_DEVICE_NUMBER 1
#endif
#ifndef MQTT_SERVER_PORT
#define MQTT_SERVER_PORT 1883
#endif
#define RETAIN_FLAG 1

#define MAX_ON_CONNECT 50

struct OnConnect {
    String topic;
    String message;
    bool retain;
};

/**************************************************************************/
/**
 * @brief This class represents a mqtt client
 * @details It connects automaticly to a mqtt server, subscibes mqtt topics
 * and handles incoming and outgoing messages
 */
/**************************************************************************/
class MqttClient : private PubSubClient {
private:
    ETHClass *_ethInterface;
    WiFiClientSecure wifiClientSecure;
    WiFiClient wifiClient;

    // Network
    bool _sslEncrypted;
    bool _gotIP = false; ///< Got IP adress provided by DHCP server
    unsigned long connectTimeStamp = 0;
    unsigned long connectInterval = 2000;

    // Mqtt broker information
    IPAddress _mqttServerIP = IPAddress(0, 0, 0, 0);
    String _domain;
    int _port = MQTT_SERVER_PORT;
    const char *_macAddress = NULL; ///< MAC address as string

    // Last Will Testament
    String _lastWillTopic;
    String _lastWillMessage;

    unsigned long ethernetTimer = 0;
    unsigned long mqttTimer = 0;

    // On connect
    std::vector<OnConnect> _publishOnConnect;
    std::vector<String> _subscribeOnConnect;
    String _networkDataTopic;

    /**************************************************************************/
    // Private Methodes
    /**************************************************************************/
    IPAddress _generateMqttServerIP();
    void _checkNetworkConnection();
    void _connectToMqtt();

public:
    MqttClient(ETHClass *ethernetClient, bool sslEncrypted = false);
    MqttClient(ETHClass *ethernetClient, bool sslEncrypted, std::function<void(char *, uint8_t *, unsigned int)> callback);

    void loop();

    // Connection settings
    void setMQTTServerIP(IPAddress mqttServerIP);
    void setMQTTServerDomain(const char *domain);
    void setPort(int port);
    void setKeepAlive(u_int16_t time);
    void setBuffer(u_int16_t size);

    void setCallback(std::function<void(char *, uint8_t *, unsigned int)> callback);
    boolean publish(const char *topic, const char *payload);
    boolean publish(const char *topic, const char *payload, bool retain);
    boolean subscribe(const char *topic);
    boolean connected();

    void publishOnConnect(String topic, String message, bool retain = true);
    void publishNetworkDataOnConnect(String topic);
    void subscribeOnConnect(String topic);

    void setLastWillMessage(String topic, String message);
    void disconnect();
    void sendLastWillMessage();

    const char *getMacAdress();
};
#endif