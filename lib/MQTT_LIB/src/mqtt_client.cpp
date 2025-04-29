/**************************************************************************/
/**
 * @file mqtt_client.cpp
 * @author Tom Meuser (t.meuser@innovaze-media.com)
 * @brief This file defines all methodes of class MqttClient which were declared
 * in mqtt_client.h
 * @version 0.1
 * @date 2024-04-15
 *
 * @copyright Copyright (c) 2024, Adunas GmbH
 *            All rights reserved.
 *            Unauthorized copying of this file, via any medium is strictly
 *            prohibited Proprietary and confidential *
 */
/**************************************************************************/
#include "logger.h"
#include <mqtt_client.h>

/**************************************************************************/
// Constructor
/**************************************************************************/
MqttClient::MqttClient(ETHClass *ethernetClient, bool sslEncrypted) : PubSubClient(), wifiClient(), wifiClientSecure() {
    _ethInterface = ethernetClient;
    _sslEncrypted = sslEncrypted;
    ethernetTimer = millis();
}

MqttClient::MqttClient(ETHClass *ethernetClient, bool sslEncrypted, std::function<void(char *, uint8_t *, unsigned int)> callback) : PubSubClient(), wifiClient(), wifiClientSecure() {
    _ethInterface = ethernetClient;
    _sslEncrypted = sslEncrypted;
    ethernetTimer = millis();
    setCallback(callback);
}

/**************************************************************************/
/**
 * @brief This function reads the local MAC adress and converts it into a
 * string.
 *
 * @return MAC adress as string
 */
/**************************************************************************/
const char *MqttClient::getMacAdress() {
    static char macChar[13];
    byte macByte[6];
    _ethInterface->macAddress(macByte);
    for (int i = 0; i < 6; i++) {
        sprintf(&macChar[i * 2], "%02X", macByte[i]);
    }
    return macChar;
}

/**************************************************************************/
/**
 * @brief This function sets the time interval of sending PING messages to the
 * mqtt broker.
 *
 * @param time Time interval [s]
 */
/**************************************************************************/
void MqttClient::setKeepAlive(u_int16_t time) {
    PubSubClient::setKeepAlive(time);
}

/**************************************************************************/
/**
 * @brief This function sets the maximum packet size for mqtt messages.
 *
 * @param size Size [byte]
 */
/**************************************************************************/
void MqttClient::setBuffer(u_int16_t size) {
    PubSubClient::setBufferSize(size);
}

/**************************************************************************/
/**
 * @brief This function sets the port for mqtt communication.
 *          Default is 1883, encrypted is 8883
 * @param port
 */
/**************************************************************************/
void MqttClient::setPort(int port) { _port = port; }

/**************************************************************************/
/**
 * @brief This function publishes a mqtt message under a specific topic
 *
 * @param topic
 * @param payload
 * @param retain
 * @return boolean
 */
/**************************************************************************/
boolean MqttClient::publish(const char *topic, const char *payload,
                            bool retain) {
    return PubSubClient::publish(topic, payload, retain);
}

/**************************************************************************/
/**
 * @brief This function sets the topic for sending the status "offline" to
 *         after the device disconnects from the network
 *
 * @param topic
 */
/**************************************************************************/
void MqttClient::setLastWillMessage(String topic, String message) {
    _lastWillTopic = topic;
    _lastWillMessage = "offline";
}

void MqttClient::disconnect() {
    PubSubClient::disconnect();
}

void MqttClient::sendLastWillMessage() {
    publish(_lastWillTopic.c_str(), _lastWillMessage.c_str());
}

void MqttClient::publishOnConnect(String topic, String message, bool retain) {
    if (_publishOnConnect.size() >= MAX_ON_CONNECT) {
        _publishOnConnect.erase(_publishOnConnect.begin());
    }
    OnConnect currentMessage = {topic, message, retain};
    _publishOnConnect.push_back(currentMessage);
}

void MqttClient::publishNetworkDataOnConnect(String topic) {
    _networkDataTopic = topic;
}

void MqttClient::subscribeOnConnect(String topic) {
    _subscribeOnConnect.push_back(topic);
}

/**************************************************************************/
/**
 * @brief This function builds the MQTT server adress using the
 *        local network adress + 1.
 */
/**************************************************************************/
IPAddress MqttClient::_generateMqttServerIP() {
    IPAddress mqttServer;
    if (_ethInterface->localIP() != IPAddress(0, 0, 0, 0)) {
        mqttServer = _ethInterface->networkID();
        mqttServer[3] += MQTT_SERVER_NETWORK_DEVICE_NUMBER;
    } else {
    }
    return mqttServer;
}

/**************************************************************************/
/**
 * @brief This function sets a static IP adress for the mqtt server
 * @details By default the mqtt server IP adress gets automaticly generated by
 the network adress + 1.

 * @param mqttServerIP
 */
/**************************************************************************/
void MqttClient::setMQTTServerIP(IPAddress mqttServerIP) {
    this->_mqttServerIP = mqttServerIP;
}

/**************************************************************************/
/**
 * @brief This function sets the domain which will be used for
 * connecting to a mqtt broker. This class prefers domain names over ip adress
 *
 * @param domain
 * @return * void
 */
/**************************************************************************/
void MqttClient::setMQTTServerDomain(const char *domain) {
    _domain = domain;
}

/**************************************************************************/
/**
 * @brief This function sets the callback function of the mqtt client.
 * @details The callback function gets executed after a message of a
 * subscribed.
 *
 * topic gets published by any client
 * @param callback callback function with three parameter (char *, uint8_t
 * *, unsigned int))
 */
/**************************************************************************/
void MqttClient::setCallback(
    std::function<void(char *, uint8_t *, unsigned int)> callback) {
    PubSubClient::setCallback(callback);
}

/**************************************************************************/
/**
 * @brief This function checks if a network connection is available
 * @details It checks if a physical connection is available and if a IP adress
 * was provided by dhcp.
 * @note This function waits until a valid connection is available.
 */
/**************************************************************************/
void MqttClient::_checkNetworkConnection() {
    // Check if physical connection and dhcp IP adress is NOT available
    if (_ethInterface->linkUp() && _ethInterface->localIP() != IPAddress(0, 0, 0, 0)) {
        _macAddress = getMacAdress();
        _gotIP = true;

        LOG.info("Connected to network " + _ethInterface->networkID().toString());

        LOG.info(String("MAC Address: " + String(_macAddress)));
        LOG.info(String("IP Address: " + _ethInterface->localIP().toString()));
        LOG.info(String("Subnetmask: " + _ethInterface->subnetMask().toString()));
        LOG.info(String("Gateway: " + _ethInterface->gatewayIP().toString()));
        LOG.info(String("DNS: " + _ethInterface->dnsIP().toString()));

        // If no server IP was set, use default one
        if (_mqttServerIP == IPAddress(0, 0, 0, 0)) {
            _mqttServerIP = _generateMqttServerIP();
        }

        // If server domain name was set, it gets prefered over server ip address
        if (_domain.isEmpty()) {
            setServer(_mqttServerIP, _port);
            LOG.info(String("Attempting connection to MQTT Broker " + _mqttServerIP.toString() + " ..."));
        } else {
            setServer(_domain.c_str(), _port);
            LOG.info(String("Attempting connection to MQTT Broker " + _domain + " ..."));
        }

        if (_sslEncrypted) {
            wifiClientSecure.setInsecure();
            setClient(wifiClientSecure);
        } else {
            setClient(wifiClient);
        }

        mqttTimer = millis();
    } else if ((millis() - ethernetTimer > ETHERNET_INIT_TIMEOUT) && !_ethInterface->linkUp()) {
        LOG.error("[CL-NC-01] Ethernet Network Connection Failed");
        ethernetTimer = millis();
    }
}

/**************************************************************************/
/**
 * @brief This function checks the network connection, intializes the mqtt
 * server connection and processes outgoing and incoming messages.
 */
/**************************************************************************/
void MqttClient::loop() {
    // Check if ethernet cable got removed
    if (!_gotIP) {
        _checkNetworkConnection();
    } else {
        if (!connected()) {
            _connectToMqtt();
        } else {
            PubSubClient::loop();
        }
        if (!_ethInterface->linkUp()) {
            LOG.error("[CL-NC-03] Lost Physical Ethernet Link");

            _gotIP = false;
            ethernetTimer = millis();
        }
    }
}

/**************************************************************************/
/**
 * @brief This function connects to the mqtt server and subscribes topics.
 */
/**************************************************************************/
void MqttClient::_connectToMqtt() {
    if (millis() - connectTimeStamp >= connectInterval) {
        if (!connected()) {
            bool isConnected = false;
            if (!_lastWillMessage.isEmpty()) {
                isConnected = connect(_macAddress, MQTT_USER, MQTT_PW, _lastWillTopic.c_str(), 0, RETAIN_FLAG, _lastWillMessage.c_str());
            } else {
                isConnected = connect(_macAddress, MQTT_USER, MQTT_PW);
            }

            if (isConnected) {

                // Publish online status
                publish(String(String(PRODUCT_TYPE) + "/" + getMacAdress() + "/" + String(MQTT_STATE)).c_str(), "online", true);

                // Published messages on connect
                if (!_publishOnConnect.empty()) {
                    for (auto &currentMessage : _publishOnConnect) {
                        PubSubClient::publish(currentMessage.topic.c_str(), currentMessage.message.c_str(), currentMessage.retain);
                    }
                    _publishOnConnect.clear();
                }

                if (_domain.isEmpty()) {
                    LOG.info(String("Connected to MQTT Broker " + _mqttServerIP.toString()));
                } else {
                    LOG.info(String("Connected to MQTT Broker " + _domain));
                }

                // Subscribe topics on connect
                if (!_subscribeOnConnect.empty()) {
                    for (String topic : _subscribeOnConnect) {
                        subscribe(topic.c_str());
                        LOG.info(String("Subscribed: " + topic));
                    }
                }

                // Published network data on connect
                if (!_networkDataTopic.isEmpty()) {
                    PubSubClient::publish(String(_networkDataTopic + "/ip_address").c_str(), _ethInterface->localIP().toString().c_str(), RETAIN_FLAG);
                    PubSubClient::publish(String(_networkDataTopic + "/subnet_mask").c_str(), _ethInterface->subnetMask().toString().c_str(), RETAIN_FLAG);
                    PubSubClient::publish(String(_networkDataTopic + "/gateway").c_str(), _ethInterface->gatewayIP().toString().c_str(), RETAIN_FLAG);
                    PubSubClient::publish(String(_networkDataTopic + "/dns").c_str(), _ethInterface->dnsIP().toString().c_str(), RETAIN_FLAG);
                }
            } else if (millis() - mqttTimer > MQTT_INIT_TIMEOUT) {
                LOG.error("[CL-NC-02] MQTT Broker Connection Failed");
                mqttTimer = millis();
            }
        }

        connectTimeStamp = millis();
    }
}

/**************************************************************************/
/**
 * @brief This function subscribes the topic defined as parameter.
 * @note It should be used only after the
 * connection to the mqtt server was already sucessufull. Otherwise use
 * setSubscribedTopic(const char *topic)
 *
 * @param topic MQtt topic
 * @return boolean
 */
/**************************************************************************/
boolean MqttClient::subscribe(const char *topic) {
    return PubSubClient::subscribe(topic);
}

/**************************************************************************/
/**
 * @brief Thsi function publishes a mqtt message under a named topic
 *
 * @param topic
 * @param payload
 * @return boolean
 */
/**************************************************************************/
boolean MqttClient::publish(const char *topic, const char *payload) {
    return PubSubClient::publish(topic, payload);
}

/**************************************************************************/
/**
 * @brief This function returns true if there is an existing connection to the mqtt
 * server
 *
 * @return boolean
 */
/**************************************************************************/
boolean MqttClient::connected() { return PubSubClient::connected(); }
