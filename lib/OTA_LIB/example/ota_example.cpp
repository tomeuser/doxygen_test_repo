/**************************************************************************/
/**
 * @file ota_example.cpp
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
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ETH.h>
#include <mqtt_client.h>
#include <ota_update.h>

#define eth_phy_addr 1
#define eth_phy_power 16
#define eth_phy_mdc 23
#define eth_phy_mdio 18
#define eth_phy_type ETH_PHY_LAN8720
#define eth_clk_mode ETH_CLOCK_GPIO0_IN

#define LOCAL_IP IPAddress(172, 20, 10, 5)
#define GATEWAY IPAddress(172, 20, 10, 57)
#define SUBNET IPAddress(255, 255, 255, 0)

#define WEBSERVER_IP IPAddress(172, 20, 10, 64)
#define PORT 4443
#define URL "firmware.bin"

#define MQTT_SERVER_IP IPAddress(172, 20, 10, 57)
#define BUTTON_1_DISPLAY "button1/display"

MqttClient mqttClient(&ETH);
OtaUpdate otaUpdate(&mqttClient);

void callback(char *topic, byte *payload, unsigned int length);

void setup() {
  Serial.begin(115200);
  ETH.begin(eth_phy_addr, eth_phy_power, eth_phy_mdc, eth_phy_mdio,
            eth_phy_type, eth_clk_mode);
  ETH.config(LOCAL_IP, GATEWAY, SUBNET);
  // mqttClient.setMQTTServerIP(MQTT_SERVER_IP);
  mqttClient.setSubscribedMacTopic(BUTTON_1_DISPLAY);
  mqttClient.setCallback(callback);

  otaUpdate.config(WEBSERVER_IP);
  // otaUpdate.config(WEBSERVER_IP, PORT, URL, false);
}

void loop() { mqttClient.loop(); }

void callback(char *topic, byte *payload, unsigned int length) {
  const char *_topic = topic;
  // Datatype conversion
  char strPayload[length + 1];  // +1 for the \0 at the end of a string
  strncpy(strPayload, reinterpret_cast<char *>(payload), length);
  strPayload[length] = '\0';

  Serial.println("New mqtt message arrived");
  Serial.println(strPayload);

  if (strcmp(_topic, mqttClient.generateMacPrefixTopic(BUTTON_1_DISPLAY)) ==
      0) {
    // Allocate JSON document and deserialize MQTT message
    DynamicJsonDocument doc(5000);
    DeserializationError error = deserializeJson(doc, strPayload);

    // Checks if payload of the MQTT-message is JSON formated
    if (error) {
      mqttClient.sendDebugMessage("messag is not JSON formated");
    } else {
      // Payload is in a valid JSON format
      otaUpdate.checkJsonOta(doc);
    }
  }
}
