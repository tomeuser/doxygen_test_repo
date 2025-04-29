# CueLight Series
## Description

The CueLight Series (CL-Series) is a portfolio of high-performance signaling devices designed to provide precise light control signals to various departments. The firmware in this repository is specifically developed for all products within the CL-Series family:

- **CL-CL1**
- **CL-CL1S**
- **CL-CL1M**
- **CL-CL1plus**
- **CL-CL2**
- **CL-CL3**
- **CL-CL4**
- **CL-CLX**
- **CL-CL16**

### Key Features:

- **Real-Time Operation**: Designed to work in real-time, ensuring immediate and precise light cues during live performances.
- **IP-Based Communication**: Utilizes IP networking, allowing seamless communication between the CueLight devices and the control desk.
- **POE Powered**: All devices are powered using Power over Ethernet (POE), reducing the need for additional power cabling and ensuring efficient, centralized power distribution.
- **Newest Addressable LEDs**: Equipped with state-of-the-art addressable LEDs, the CueLight devices can produce vibrant and customizable lighting effects.

## Table of Contents
- [Flashing Firmware on Micro Controller](#flashing-firmware-on-micro-controller)
- [Supported Hardware](#supported-hardware)
- [Getting Started for Development](#getting-started-for-development)
- [Used Libraries](#used-libraries)
- [Capabilities](#capabilities)
- [Documentation](#documentation)
- [Contributing](#contributing)
- [License](#license)


## Supported Hardware

The CueLight Series firmware is designed to work with Espressif ESP32-based boards. It was primarily developed for the **Adunas Base Circuit Board**, which features following components:
- Espressif [ESP32-WROOM-32D](https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32d_esp32-wroom-32u_datasheet_en.pdf)
- Gigabit Ethernet 100/1000 Base-T ([G2406S](https://wmsc.lcsc.com/wmsc/upload/file/pdf/v2/lcsc/2312221708_CND-tek-G2406S_C408892.pdf))
- Power over Ethernet Module ([Ag5324](https://silvertel.com/ag5300/) / [Ag5800](https://silvertel.com/ag5800/))
- I2C I/O Expander ([PCA9535](https://www.ti.com/lit/ds/symlink/pca9535.pdf))
- Temperature & Pressure Sensor ([BMP388](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bmp388-ds001.pdf))


Additionally, the Adunas Base Circuit Board can be connected with product-dependent extension boards, including:

### LED Matrix Panel
Designed for CL-CL1, CL-CL1S, CL-CLM, CL-CL1plus, CL-CL2, CL-CL3, CL-CL4, CLX, it features following components:
- 16x7 RGB addressable LEDs ([WS2812B](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf))
- 16-Bit Analog-to-Digital Converter([ADS1115](https://cdn-shop.adafruit.com/datasheets/ads1115.pdf))
- Photoconductive Cells ([GL5528](https://cdn.sparkfun.com/datasheets/Sensors/LightImaging/SEN-09088.pdf))


### LoRa Button Extension
Designed for CL-CLX, it features following components:
- 1x RGB addressable LED ([WS2812B](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf))
- Broadcast Button
- LoRaWAN Module ([LA66](https://www.dragino.com/products/lora/item/230-la66-lorawan-module.html))

### Control X8 Extension
Designed for CL-CL16, it features following components:
- I2C I/O Expander ([PCA9535](https://www.ti.com/lit/ds/symlink/pca9535.pdf))


## Flashing Firmware on Micro Controller

There are three methods to flash the CueLight Series firmware onto the board:

### 1. Flashing Over the Air (OTA)
This method allows flashing the firmware wirelessly through the **Adunas Debug Console**. Before proceeding, ensure the following are properly configured:
- Adunas MQTT Broker 
- Adunas Debug Console is running in a Node-Red instance and is connected to the Adunas MQTT Broker

![Adunas Debug Console](/docs/images/adunas_debug_console.png)


### 2. Flashing via Espressif Flash Tool
The **Espressif Flash Tool** allows you to flash firmware to your board via a wired connection. You’ll need the bootloader, partition table, and firmware binary files, which can be downloaded from the [latest release](#) node of this repository.

#### Steps to Flash:
1. Download the following files from the [latest release](#):
   - Bootloader
   - Partition Table
   - Firmware File
2. Install the **Espressif Flash Tool** from [Espressif](https://www.espressif.com/sites/default/files/tools/flash_download_tool_3.9.7_2.zip).
3. Connect your board to your computer using a USB Programmer Clip.
4. Open the Espressif Flash Tool and choose Chip Type ESP32 and Work Mode Development 
5. Select the appropriate files and memory addresses:
   - **Bootloader**: Select the bootloader file and set the address to `0x1000`.
   - **Partition Table**: Select the partition table file and set the address to `0x8000`.
   - **Firmware**: Select the firmware file and set the address to `0x10000`.
5. Choose the correct COM port for your device and set the baud rate to `115200`.
6. Click **Start** to begin the flashing process.

![Espressif Flash Tool Setup](/docs/images/espressif_flash_tool.png)


### 3. Flashing through Visual Studio Code with PlatformIO
You can also flash the firmware using **Visual Studio Code** with the **PlatformIO extension**. Before flashing, ensure that you’ve set up the development environment as described in the [Getting Started for Development](#getting-started-for-development) section. 

Once set up, simply use the following steps:
1. Open the project in Visual Studio Code.
2. Build the project using PlatformIO.
3. Connect your board and click **Upload** to flash the firmware.

![Flash Firmware with Visual Studio Code ](/docs/images/vs-code_flash_fw.png)


## Getting Started for Development

To get started with the development and installation of the CueLight Series firmware, you will need the following tools installed on your system:

### 1. **Visual Studio Code**

Visual Studio Code is the preferred Integrated Development Environment (IDE) for working with this project. You can download and install it from the following link:

- [Download Visual Studio Code](https://code.visualstudio.com/download)

### 2. **PlatformIO Extension**

The PlatformIO extension is required to build and upload the firmware to the devices. It integrates seamlessly with Visual Studio Code. You can install it by following the steps provided by PlatformIO:

- [Install PlatformIO](https://platformio.org/install/ide?install=vscode)

### 3. **Git**

Git is required to clone the repository and in  order to continue development and manage version control. If Git is not installed on your machine, you can download and install it from the official website:

- [Download Git](https://git-scm.com/downloads)

Once installed, you can verify the installation by opening a terminal and running:
```bash
git --version
```

### 4. **Doxygen Documentation Generator Extension**

The Doxygen Documentation Generator extension allows you to generate documentation for the firmware. This is helpful for anyone who wants to modify the code or continue development. You can install the extention directly from the following link:

- [Install Doxygen Documentation Generator](https://marketplace.visualstudio.com/items?itemName=cschlosser.doxdocgen)

If Doxygen is not already installed on your system, you can download and install it from the official website:

- [Download Doxygen](https://www.doxygen.nl/download.html)

### 5. **Clone the Git Repository**

You can obtain the project by either cloning the Git repository or downloading it as a `.zip` file.
#### Option 1: Clone the Repository
1. Open Visual Studio Code.
2. Open the command palette by pressing `Ctrl+Shift+P`.
3. Type `Git: Clone` and select the **Git: Clone** option.
4. When prompted, enter the repository URL:

```bash
git clone https://gitlab.adunas.cloud/adunas-embedded-developers/CueLight.git
```

 5. Select the folder where you want to save the cloned repository.

 #### Option 2: Download as .zip

If you prefer not to use Git, you can download the repository as a `.zip` file:

1. Click the Code button, then click **Download ZIP**.
2. Extract the `.zip`  file to a folder on your machine.


## Used Libraries

The following open-source libraries are utilized in the CueLight Series firmware:

- [PubSubClient](https://pubsubclient.knolleary.net/) - A client library for MQTT.
- [ArduinoJson](https://arduinojson.org/) - A library to serialize and deserialize JSON data.
- [Adafruit BMP3XX](https://github.com/adafruit/Adafruit_BMP3XX) - Library for the BMP388 temperature and pressure sensor.
- [PCA95x5](https://github.com/hideakitai/PCA95x5) - Library for the PCA95x5 I/O expander.
- [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) - Library for controlling addressable LEDs.
- [Adafruit ADS1X15](https://github.com/adafruit/Adafruit_ADS1X15) - Library for the ADS1X15 ADC.
- [ArduinoOTA](https://github.com/JAndrassy/ArduinoOTA) - Library for Over-The-Air (OTA) updates.
- [ArduinoHttpClient](https://github.com/arduino-libraries/ArduinoHttpClient) - Library for making HTTP requests.

These libraries were used within the Arduino framework v2.0.17, which is based on Espressif-IDF v4.4.7.

## Capabilities

The CueLight Series firmware currently supports the following functionalities:
### Common Features
- **Logging**: Comprehensive logging capabilities for diagnostics and monitoring.
- **Temperature Monitoring**: Prevents overheating by monitoring temperature levels.  
- **Over-The-Air (OTA) Updates**: Allows firmware updates without physical access.

### Product-Specific Features

- **CL1, CL2, CL3, CL4, CL1S, CL1M, CL1plus, CL16**
  - **IP-based Control via MQTT**: Enables seamless control over the network.
  - **POE Type Detection**: Protects devices by limiting brightness if POE type is too low.

- **CL1, CL2, CL3, CL4, CL1S, CL1M, CL1plus, CLX**
  - **LED Matrix Control**: Features include On, Off, Blink, and Set Color.
  - **Matrix Error Detection**: Utilizes photoconductive cells to ensure the matrix is functioning as commanded.

- **CL1plus, CLX**
  - **Acknowledgment Button**: Provides feedback for operations, complemented by a button LED.

- **CLX**
  - **LoRaWAN Communication**: Enables long-range, low-power communication.

- **CL16**
  - **Controller for Inputs/Outputs**: Manages 16 inputs and switches 16 outputs, compatible with 24V operating CueLights.


## Documentation

Doxygen [Documentation]() is provided to assist in understanding the codebase and facilitate further development. 


## Contributing

This repository was primarily developed and maintained by [Tom Meuser](mailto:t.meuser@innovaze-media.com) and [Daniel Ahlers](mailto:d.ahlers@innovaze-media.com).

## License

Copyright (c) 2024, Adunas GmbH. All rights reserved.

This software is proprietary and confidential. Unauthorized copying of this repository, in any medium, is strictly prohibited. Use of this software is subject to the terms and conditions set forth by Adunas GmbH.
