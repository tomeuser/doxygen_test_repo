# OTA_LIB

OTA_LIB is a library designed to facilitate over-the-air (OTA) firmware updates for ESP32. It integrates seamlessly with MQTT_LIB to handle MQTT communications and supports SSL/TLS for secure firmware transmission.

## Features
- **Secure Firmware Updates**: Implements SSL/TLS to ensure secure transmission of firmware files.
- **MQTT Integration**: Works in conjunction with MQTT_LIB to enable MQTT messaging for update triggers and to publish debug messages .
- **Dynamic Configuration**: Configure IP address, port, and URL of the firmware server at runtime, either automaticcly with default values or manually.
- **JSON Trigger Support**: Checks incoming MQTT messages for OTA update triggers formatted in JSON.

## Dependencies
This OTA_LIB Library relies on the following external libraries for its operation:

1. Clone the repository or download the latest release.
2. Ensure that the following dependencies are installed:
   - [`ArduinoJson`](https://github.com/arduino-libraries/Arduino_JSON)
   - [`SSLClient`](https://github.com/OPEnSLab-OSU/SSLClient/)
   - [`MQTT_LIB`](https://github.com/tomeuser/MQTT_LIB)

## Documentation
For detailed documentation of the library, see the [Doxygen Documentation](/docs/html/index.html).


## Prerequisites
Before you start, make sure you have the following:
- An ESP32 development board.
- The PlatformIO extention for VS code
- ESP32 board configured with an Ethernet or similar network connection setup.

## Installation
1. Download this library from the GitHub repository.
2. Place this library in the `/lib` directory of your project

## Usage Example
`/example` provides different examples on how to use this library


## License
Copyright (c) 2024, Adunas GmbH. All rights reserved.

Unauthorized copying of this file, via any medium is strictly prohibited. Proprietary and confidential.
