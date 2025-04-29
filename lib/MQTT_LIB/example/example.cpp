/**************************************************************************/
/**
 * @file example.cpp
 * @author Tom Meuser (t.meuser@innovaze-media.com)
 * @brief This file provides an example for how to use this library
 * @version 0.1
 * @date 2024-04-15
 *
 * @copyright Copyright (c) 2024, Adunas GmbH
 *            All rights reserved.
 *            Unauthorized copying of this file, via any medium is strictly
 *            prohibited Proprietary and confidential *
 */
/**************************************************************************/
#include <mqtt_client.h>

// Define callback function
void callback(char *topic, byte *payload, unsigned int length);

// Create an instance of MqttClient
MqttClient mqttClient(&ETH, callback);
// Optional:
// MqttClient mqttClient(&ETH);

// Define mqtt topics to subscribe
#define SINGLE_TOPIC "single_test"

const int TOPIC_COUNT = 3;
#define TOPIC_1 "api"
#define TOPIC_2 "debug"
#define TOPIC_3 "test"

// These topics will get the mac adress added as prefix
#define SINGLE_MAC_TOPIC "single_mac_test"  // [MAC]/single_mac_test

const int MAC_TOPIC_COUNT = 3;
#define MAC_TOPIC_1 "api"    // will look like [MAC]/api
#define MAC_TOPIC_2 "debug"  // will look like [MAC]/debug
#define MAC_TOPIC_3 "test"   // will look like [MAC]/test

// Store topics in an array
const char *macTopics[MAC_TOPIC_COUNT] = {MAC_TOPIC_1, MAC_TOPIC_2,
                                          MAC_TOPIC_3};
const char *topics[TOPIC_COUNT] = {TOPIC_1, TOPIC_2, TOPIC_3};

// interface to Ethernet PHY (LAN8720A)
static const uint8_t eth_phy_addr = 1;
static const uint8_t eth_phy_power = 05;
static const uint8_t eth_phy_mdc = 23;
static const uint8_t eth_phy_mdio = 18;
static const eth_phy_type_t eth_phy_type = ETH_PHY_LAN8720;
static const eth_clock_mode_t eth_clk_mode = ETH_CLOCK_GPIO0_IN;

#define LOCAL_IP IPAddress(172, 20, 10, 5)
#define GATEWAY IPAddress(172, 20, 10, 57)
#define SUBNET IPAddress(255, 255, 255, 0)

void setup() {
  // Inizialize serial communication
  Serial.begin(115200);
  // Inizialize ethernet interface
  ETH.begin(eth_phy_addr, eth_phy_power, eth_phy_mdc, eth_phy_mdio,
            eth_phy_type, eth_clk_mode);

  // Subscribe previos defined mqtt topics
  mqttClient.setSubscribedTopic(SINGLE_TOPIC);         // Single topic
  mqttClient.setSubscribedTopic(topics, TOPIC_COUNT);  // Topic array

  // Subscribe previous defined mqtt topics and add mac adress as prefix
  mqttClient.setSubscribedMacTopic(SINGLE_MAC_TOPIC);            // Single topic
  mqttClient.setSubscribedMacTopic(macTopics, MAC_TOPIC_COUNT);  // Topic array

  // Optional:
  // mqttClient.setCallback(callback);
  // mqttClient.setMQTTServerIP(IPAddress(172, 20, 10, 1));
  // mqttClient.setMqttServerLastOctet(0x01);
}
bool published = false;

void loop() {
  mqttClient.loop();

  // Publish one example message
  if (mqttClient.connected() && !published) {
    mqttClient.publish(TOPIC_1, "Hello World!");

    // Publish under a topic with mac adress prefix
    mqttClient.publish(mqttClient.generateMacPrefixTopic(MAC_TOPIC_1),
                       "Hello World!");
    published = true;
  }
}

void callback(char *topic, byte *payload, unsigned int length) {
  // define action which will be executed after a message from a subscribed
  // topic will arrive

  // Optional:
  // mqttClient.sendDebugMessage("Message arrived!");
}
