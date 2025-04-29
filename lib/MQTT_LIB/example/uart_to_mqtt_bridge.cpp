/**
 * @file main.cpp
 * @author Tom Meuser (t.meuser@innovaze-media.com)
 * @brief Skatch to convert MQTT messages to UART messages and reverse.
 * Initializes an encrypted MQTT Client and an UART interface.
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <mqtt_client.h>

// UART
void uartToMqtt(HardwareSerial &uart, MqttClient &client);
void mqttToUart(char *topic, byte *payload, unsigned int length);

HardwareSerial SerialPort(2);
String receivedMessage;
MqttClient mqttClient(&ETH, mqttToUart);

#define TOPIC_COUNT 2
#define RX_TOPIC "Rx"
#define TX_TOPIC "Tx"

const char *topics[TOPIC_COUNT] = {RX_TOPIC, TX_TOPIC};

// interface to Ethernet PHY (LAN8720A)
static const uint8_t eth_phy_addr = 1;
static const uint8_t eth_phy_power = 05;
static const uint8_t eth_phy_mdc = 23;
static const uint8_t eth_phy_mdio = 18;
static const eth_phy_type_t eth_phy_type = ETH_PHY_LAN8720;
static const eth_clock_mode_t eth_clk_mode = ETH_CLOCK_GPIO0_IN;

/**************************************************************************/
/**
 * @brief This function defines actions before the main loop gets started.
 */
/**************************************************************************/
void setup() {
  Serial.begin(115200);
  SerialPort.begin(9600, SERIAL_8N1, 16, 17);
  SerialPort.setTimeout(300);
  Serial.println();
  Serial.println();

  ETH.begin(eth_phy_addr, eth_phy_power, eth_phy_mdc, eth_phy_mdio,
            eth_phy_type, eth_clk_mode);
  mqttClient.setMQTTServerIP(IPAddress(172, 20, 10, 1));
  mqttClient.setSubscribedMacTopic(topics, TOPIC_COUNT);
}

/**************************************************************************/
/**
 * @brief This function is the main loop of this software.
 * @details
 */
/**************************************************************************/
void loop() {
  uartToMqtt(SerialPort, mqttClient);
  mqttClient.loop();
}

/**
 * @brief Converts a receving UART message into a MQTT message.
 *
 * @param uart Reference to UART interface
 * @param client Reference to MQTT-Client
 */
void uartToMqtt(HardwareSerial &uart, MqttClient &client) {
  if (uart.available() > 0) {
    client.sendDebugMessage("UART message available:");
    receivedMessage = "";  // Reset the received message
    receivedMessage = uart.readStringUntil('\n');
    Serial.println(receivedMessage);
    if (receivedMessage != "") {
      client.publish(client.generateMacPrefixTopic("Tx"),
                     receivedMessage.c_str());
    }
    client.sendDebugMessage("UART converted to MQTT sucessfully");
    Serial.println();
  }
}

/**
 * @brief Callback function for the MQTT client to receive MQTT message and
 * convert it into a UART message.
 *
 * @param topic MQTT-topic of which the arriving MQTT message got published
 * @param payload Payload of the arriving MQTT message
 * @param length Payload length
 */
void mqttToUart(char *topic, byte *payload, unsigned int length) {
  // convert byte into char array
  char strPayload[length + 1];  // +1 for the \0 at the end of a string
  strncpy(strPayload, reinterpret_cast<char *>(payload), length);
  strPayload[length] = '\0';
  // sendDebugMessage("MQTT Message available:");
  mqttClient.sendDebugMessage("MQTT message available:");
  // Serial.println("MQTT message available:");
  mqttClient.sendDebugMessage(strPayload);

  // Serial.println(strPayload);
  //  MQTT to UART
  SerialPort.println(strPayload);
  // sendDebugMessage("MQTT converted to UART sucessfully");
}
