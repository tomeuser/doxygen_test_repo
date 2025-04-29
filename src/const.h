/**************************************************************************/
/**
 * @file const.h
 * @author Tom Meuser (t.meuser@innovaze-media.com),
 *         Daniel Ahlers (d.ahlers@innovaze-media.com)
 * @brief Project settings
 * @version 1.0.3
 * @date 2024-11-28
 *
 * @copyright Copyright (c) 2024, Adunas GmbH
 *            All rights reserved.
 *            Unauthorized copying of this file, via any medium is strictly prohibited
 *            Proprietary and confidential
 *
 */
/**************************************************************************/

// Please be careful with any sort of changes!!

#ifdef AD_CL_CL1
#define PRODUCT_TYPE "AD-CL-CL1-1"
#define LED_MATRIX_COUNT 1    // Amount of physical led matrix tiles
#define CL_CHAMBER_COUNT 1    // Amount of theoretical chambers
#define CL_CHAMBER_WIDTH {16} // Width of each chamber (has to be as many values as CL_CHAMBER_COUNT indicates)
#elif defined(AD_CL_CL16)
#define PRODUCT_TYPE "AD-CL-CL16-1"
#define CL_CHAMBER_COUNT 16
#define LED_MATRIX_COUNT 0
#define BLINK_INTERVALE 2000
#elif defined(AD_CL_CLX)
#define PRODUCT_TYPE "AD-CL-CLX-1"
#define LED_MATRIX_COUNT 1
#define CL_CHAMBER_COUNT 1
#define CL_CHAMBER_WIDTH {16}
#define ACK_BUTTON
#elif defined(AD_CL_CL1_PLUS)
#define PRODUCT_TYPE "AD-CL-CL1plus-1"
#define LED_MATRIX_COUNT 1
#define CL_CHAMBER_COUNT 1
#define CL_CHAMBER_WIDTH {16}
#define ACK_BUTTON
#elif defined(AD_CL_CL1S)
#define PRODUCT_TYPE "AD-CL-CL1S-1"
#define LED_MATRIX_COUNT 1
#define CL_CHAMBER_COUNT 3
#define CL_CHAMBER_WIDTH {5, 6, 5}
#elif defined(AD_CL_CL2)
#define PRODUCT_TYPE "AD-CL-CL2-1"
#define LED_MATRIX_COUNT 2
#define CL_CHAMBER_COUNT 2
#define CL_CHAMBER_WIDTH {16, 16}
#elif defined(AD_CL_CL3)
#define PRODUCT_TYPE "AD-CL-CL3-1"
#define LED_MATRIX_COUNT 3
#define CL_CHAMBER_COUNT 3
#define CL_CHAMBER_WIDTH {16, 16, 16}
#elif defined(AD_CL_CL4)
#define PRODUCT_TYPE "AD-CL-CL4-1"
#define LED_MATRIX_COUNT 4
#define CL_CHAMBER_COUNT 4
#define CL_CHAMBER_WIDTH {16, 16, 16, 16}
#endif

// Matrix Controller
#ifndef LED_MATRIX_WIDTH
#define LED_MATRIX_WIDTH 16
#endif
#ifndef LED_MATRIX_HEIGHT
#define LED_MATRIX_HEIGHT 7
#endif

// MQTT settings
#ifndef MQTT_HOST_NAME
#define MQTT_HOST_NAME "mqtt.adunas.cloud"
#endif
#ifndef MQTT_HOST_IP
#define MQTT_HOST_IP IPAddress(192, 168, 178, 20)
#endif
#ifndef MQTT_SERVER_PORT
#define MQTT_SERVER_PORT 8883
#endif
#ifndef MQTT_USER
#define MQTT_USER "device_user"
#endif
#ifndef MQTT_PW
#define MQTT_PW "4MXjj~qh2&"
#endif
#ifndef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 4096
#endif
#ifndef MQTT_PING_INTERVAL
#define MQTT_PING_INTERVAL 5
#endif

// MQTT topics
#ifndef MQTT_CONFIG
#define MQTT_CONFIG "config"
#endif
#ifndef MQTT_OTA_CHECK
#define MQTT_OTA_CHECK "ota/check"
#endif
#ifndef MQTT_OTA_START
#define MQTT_OTA_START "ota/start"
#endif
#ifndef MQTT_OTA_AVAILABLE
#define MQTT_OTA_AVAILABLE "ota/available"
#endif
#ifndef MQTT_INFO
#define MQTT_INFO "info"
#endif
#ifndef MQTT_STATE
#define MQTT_STATE "state"
#endif
#ifndef MQTT_CONTROL
#define MQTT_CONTROL "control"
#endif
#define MQTT_INIT_TIMEOUT 10000

// Ota settings
#ifndef OTA_VERSION_FILE
#define OTA_VERSION_FILE "https://ota.adunas.cloud/remote.php/dav/files/device_user/ADUNAS-Firmware/latest_fw.json"
#endif
#ifndef OTA_USER_NAME
#define OTA_USER_NAME "device_user"
#endif
#ifndef OTA_PASSWORD
#define OTA_PASSWORD "4MXjj~qh2&"
#endif
#ifndef OTA_HTTP_TIMEOUT
#define OTA_HTTP_TIMEOUT 15000
#endif

/**************************************************************************/
// Interfaces - Pin layout
/**************************************************************************/

// Ethernet interface
#ifndef ETH_PHY_ADDR
#define ETH_PHY_ADDR 1
#endif
#ifndef ETH_PHY_POWER
#define ETH_PHY_POWER 05
#endif
#ifndef ETH_PHY_MDC
#define ETH_PHY_MDC 23
#endif
#ifndef ETH_PHY_MDIO
#define ETH_PHY_MDIO 18
#endif
#ifndef ETH_PHY_TYPE
#define ETH_PHY_TYPE ETH_PHY_LAN8720
#endif
#ifndef ETH_CLK_MODE
#define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN
#endif
#ifndef ETHERNET_INIT_TIMEOUT
#define ETHERNET_INIT_TIMEOUT 20000
#endif

// I2C interface
#ifndef I2C_SDA_PIN
#define I2C_SDA_PIN 32
#endif
#ifndef I2C_SCL_PIN
#define I2C_SCL_PIN 33
#endif
#ifndef I2C_FREQ
#define I2C_FREQ 400000
#endif
#ifndef I2C_ON_BOARD_MULTIPLEXER_ADDR
#define I2C_ON_BOARD_MULTIPLEXER_ADDR 0x20
#endif
#ifndef I2C_EXTERNAL_IO_MULTIPLEXER_ADDR_1
#define I2C_EXTERNAL_IO_MULTIPLEXER_ADDR_1 0x21
#endif
#ifndef I2C_EXTERNAL_IO_MULTIPLEXER_ADDR_2
#define I2C_EXTERNAL_IO_MULTIPLEXER_ADDR_2 0x23
#endif
#ifndef I2C_EXTERNAL_POWER_SENSOR_ADDR_1
#define I2C_EXTERNAL_POWER_SENSOR_ADDR_1 0x40
#endif
#ifndef I2C_EXTERNAL_POWER_SENSOR_ADDR_2
#define I2C_EXTERNAL_POWER_SENSOR_ADDR_2 0x41
#endif

#define EXT_MULTIPLEXER_COUNT 2
#define EXT_MULTIPLEXER_IO_OUTPUTS 8
#define EXT_MULTIPLEXER_IO_INPUTS 8

// LED matrix
#ifndef LED_MATRIX_PIN
#define LED_MATRIX_PIN 4
#endif
#ifndef MATRIX_POWER_PIN
#define MATRIX_POWER_PIN 15
#endif
#ifndef ACK_BUTTON_PIN
#define ACK_BUTTON_PIN 9
#endif

/**************************************************************************/
// Constants
/**************************************************************************/

// Memory manager
#ifndef MEMORY_NAMESPACE
#define MEMORY_NAMESPACE "config"
#endif
#ifndef DEFAULT_DHCP
#define DEFAULT_DHCP true // the following 4 settings will just apply if DEFAULT_DHCP is set to false
#endif
#ifndef DEFAULT_LOCAL_IP
#define DEFAULT_LOCAL_IP "192.168.0.10"
#endif
#ifndef DEFAULT_SUBNET_MASK
#define DEFAULT_SUBNET_MASK "255.255.255.0"
#endif
#ifndef DEFAULT_GATEWAY
#define DEFAULT_GATEWAY "192.168.0.1"
#endif
#ifndef DEFAULT_DNS
#define DEFAULT_DNS "192.168.0.1"
#endif

// Matrix Controller
#ifndef BLINK_INTERVALE
#define BLINK_INTERVALE 1000
#endif
#ifndef READ_INTERVALE
#define READ_INTERVALE 2 // [ms] time between all buttons are read
#endif
#ifndef MAXIMUM_DEBOUNCE_TIME
#define MAXIMUM_DEBOUNCE_TIME 10 // [ms] needs to be dividable by the READ_INTERVALE
#endif
#ifndef MAXIMUM_DEBOUNCE_COUNTER
#define MAXIMUM_DEBOUNCE_COUNTER (MAXIMUM_DEBOUNCE_TIME / READ_INTERVALE)
#endif

// Brightness limits
#ifndef MAX_BRIGHTNESS
#define MAX_BRIGHTNESS 210
#endif
#ifndef LOW_ENERGY_BRIGHTNESS
#define LOW_ENERGY_BRIGHTNESS 32
#endif

// Default color
#ifndef DEFAULT_RED
#define DEFAULT_RED 0
#endif
#ifndef DEFAULT_GREEN
#define DEFAULT_GREEN 0
#endif
#ifndef DEFAULT_BLUE
#define DEFAULT_BLUE 0
#endif

// Temperature sensor
#ifndef OVERHEAT_START
#define OVERHEAT_START 85
#endif
#ifndef OVERHEAT_STOP
#define OVERHEAT_STOP 75
#endif

#ifndef TEMP_READ_INTERVAL
#define TEMP_READ_INTERVAL 60000
#endif

// Light Sensor
#ifndef LS_I2C_ADDRESS_CL1
#define LS_I2C_ADDRESS_CL1 0x48
#endif
#ifndef LS_I2C_ADDRESS_CL2
#define LS_I2C_ADDRESS_CL2 0x49
#endif
#ifndef LS_I2C_ADDRESS_CL3
#define LS_I2C_ADDRESS_CL3 0x4A
#endif
#ifndef LS_I2C_ADDRESS_CL4
#define LS_I2C_ADDRESS_CL4 0x4B
#endif
#ifndef LS_ADC_GAIN
#define LS_ADC_GAIN GAIN_ONE // Setting these values incorrectly may destroy your ADC
#endif
#ifndef LS_INIT_TIMEOUT
#define LS_INIT_TIMEOUT 5000 // [ms]
#endif
#ifndef LS_READ_INTERVAL
#define LS_READ_INTERVAL 500 // [ms]
#endif
#ifndef LS_SWITCH_STATE_INDICATOR
#define LS_SWITCH_STATE_INDICATOR 20
#endif

// POE module
#ifndef POE_READ_INTERVAL
#define POE_READ_INTERVAL 5000
#endif

/**************************************************************************/
// Debug tools
/**************************************************************************/
#define SSL

#ifndef SSL
#undef MQTT_SERVER_PORT
#define MQTT_SERVER_PORT 1883
#endif
