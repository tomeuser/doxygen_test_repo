/**************************************************************************/
/**
 * @file ota_update.h
 * @author Tom Meuser (t.meuser@innovaze-media.com)
 * @brief This is the headerfile of the class OtaUpdate declaring all varibales
 * and methods used by this class
 * @version 0.1
 * @date 2024-04-15
 *
 * @copyright Copyright (c) 2024, Adunas GmbH
 *            All rights reserved.
 *            Unauthorized copying of this file, via any medium is strictly
 *            prohibited Proprietary and confidential *
 */
/**************************************************************************/
#ifndef OTA_UPDATE_H
#define OTA_UPDATE_H

#include <ArduinoJson.h>
#include <mqtt_client.h>

#include "Client.h"
#include "HttpClient/HttpClient.h"
#include "InternalStorage/InternalStorageESP.h"
#include "logger.h"

#define HTTP_PORT 80
#define HTTPS_PORT 443
#ifndef PRODUCT_TYPE
#define PRODUCT_TYPE ""
#endif
#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "0.0.0"
#endif

class OtaUpdate {
private:
    // Class instances
    MqttClient *_mqttClient;
    Client *_client;
    HttpClient httpClient;
    InternalStorageESPClass InternalStorage;
    String newFirmwareUrl;
    ETHClass *_eth;
    String fwVersionFileUrl;
    String _topic;

    // request json file
    bool
    requestLatestFirmware(const char *url);
    void splitUrl(String url, String &domain, unsigned short &port, String &path);
    void readJson(String jsonMessage);
    bool compareFwVersion(const char *oldFw, const char *newFw);
    bool isFwAvailable();

    // request firmware
    bool _handleOTA(HttpClient *httpClient, const char *host, const char *path);
    bool requestFirmware(IPAddress ip, unsigned short port, const char *path);
    bool requestFirmware(const char *domain, unsigned short port, const char *path);

public:
    // Constructur
    OtaUpdate(ETHClass *eth, Client *client, MqttClient *mqttClient);

    void checkJsonOta(DynamicJsonDocument doc);
    void setFwVersionFileUrl(const char *fileUrl);

    void startOfficialUpdate();
    void startOfficialVersionRequest();
    void setUpdateAvailableTopic(const char *topic);
};

#endif
