# MQTT Client Library for ESP32

## Introduction
This MQTT Client Library provides a comprehensive solution for connecting ESP32 devices to MQTT brokers securely over SSL/TLS. Designed with simplicity in mind, it offers a straightforward way for devices to publish and subscribe to topics in the MQTT protocol within the Arduino framework.

## Features
- **SSL/TLS Encryption**: Secure your MQTT communications using SSL/TLS.
- **Automatic Connection Management**: Automatically handles MQTT server connections, including reconnections.
- **Multiple Topic Subscriptions**: Subscribe to multiple topics and handle messages efficiently.
- **Easy Message Publication**: Publish messages to MQTT topics with minimal code.
- **Ethernet Integration**: Utilizes the Ethernet library to manage network communication.
- **Callback Support**: Execute custom code upon receiving messages through subscribed topics.

## Dependencies

This MQTT Client Library relies on the following external libraries for its operation:

- **PubSubClient**: A client library for MQTT messaging. It supports publishing, subscribing, and receiving messages. Visit the PubSubClient GitHub repository [here](https://github.com/knolleary/pubsubclient).

- **SSLClient**: This library wraps around existing Arduino libraries to add SSL/TLS functionality, enabling secure connections for various Internet protocols. Check out the SSLClient GitHub repository [here](https://github.com/OPEnSLab-OSU/SSLClient/).

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
3. If you want to use SSL encryption, make sure to create a certificate file `certificates.h` and replace the one provided in `/src`. You can find more information on how tyo create this certificate under the following link [here](https://github.com/OPEnSLab-OSU/SSLClient/)
    

## Usage Example
`/example` provides different examples on how to use this library

## API Methods

The library includes several methods for MQTT client configuration and operation:

- `setCallback()`: Assigns a callback function for message handling.
- `setSubscribedTopic()`, `setSubscribedMacTopic()`: Subscribe to topics with or without the device's MAC address as a prefix.
- `publish()`: Publishes a message to a specified topic.
- `loop()`: Maintains the connection and handles incoming and outgoing messages.

Refer to `mqtt_client.h` for a complete list of available methods and their documentation.

## Configuration

To switch between encrypted and unencrypted connections, use the method `enableSSL()`. The `certificates.h` file must contain your MQTT broker's certificate information for SSL/TLS encryption. To avoid errors, it is crucial to change one line of code in the `SSLClient.h` file of the SSLClient library. 
For a more detailed explanation check out the following issue: [Discarded unread data to favor a write operation](https://github.com/OPEnSLab-OSU/SSLClient/issues/46)

Change line 470 from:

```c
unsigned char m_iobuf[2048];
```
into:

```c
unsigned char m_iobuf[BR_SSL_BUFSIZE_BIDI];
```


## License

Copyright (c) 2024, Adunas GmbH
All rights reserved.
Unauthorized copying of this file, via any medium is strictly
prohibited Proprietary and confidential *
