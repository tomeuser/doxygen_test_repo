/**************************************************************************/
/**
 * @file ota_update.cpp
 * @author Tom Meuser (t.meuser@innovaze-media.com)
 * @brief This file defines all methodes of class OtaUpdate which were declared
 * in ota_update.h
 * @version 0.1
 * @date 2024-04-15
 *
 * @copyright Copyright (c) 2024, Adunas GmbH
 *            All rights reserved.
 *            Unauthorized copying of this file, via any medium is strictly
 *            prohibited Proprietary and confidential *
 */
/**************************************************************************/
#include <ota_update.h>

/**************************************************************************/
/**
 * @brief Construct a new Ota Update:: Ota Update object
 *
 * @param eth Network interface to check connectivity status
 * @param client Client for http communication
 * @param mqttClient Mqtt client
 */
/**************************************************************************/
OtaUpdate::OtaUpdate(ETHClass *eth, Client *client, MqttClient *mqttClient) : httpClient(*client) {
    _eth = eth;
    _mqttClient = mqttClient;
    _client = client;
}

bool OtaUpdate::requestFirmware(IPAddress ip, unsigned short port, const char *path) {
    LOG.info("Begin with OTA Update..");
    _mqttClient->loop();

    httpClient = HttpClient(*_client, ip, port);
    httpClient.setHttpResponseTimeout(OTA_HTTP_TIMEOUT);
    httpClient.setTimeout(OTA_HTTP_TIMEOUT);
    LOG.info(String("HTTP-Request " + ip.toString() + ":" + String(port) + "/" + String(path)));

    return _handleOTA(&httpClient, ip.toString().c_str(), path);
}

bool OtaUpdate::requestFirmware(const char *domain, unsigned short port, const char *path) {
    LOG.info("Begin with OTA Update..");
    _mqttClient->loop();

    httpClient = HttpClient(*_client, domain, port);
    httpClient.setHttpResponseTimeout(OTA_HTTP_TIMEOUT);
    httpClient.setTimeout(OTA_HTTP_TIMEOUT);

    LOG.info(String("HTTP-Request " + String(domain) + ":" + String(port) + "/" + String(path)));

    return _handleOTA(&httpClient, domain, path);
}

/**************************************************************************/
/**
 * @brief This function is a help function to analyse a json object if it
 * contains an 'ota' key.
 *
 * @param doc Json formated message (json document)
 */
/**************************************************************************/
void OtaUpdate::checkJsonOta(DynamicJsonDocument doc) {
    if (doc.containsKey("ota")) {
        bool containsObject = false;

        if (doc["ota"].containsKey("url")) {
            String url = doc["ota"]["url"].as<const char *>();
            String domain, path;
            unsigned short port;
            splitUrl(url, domain, port, path);
            requestFirmware(domain.c_str(), port, path.c_str());
        }
    }
}

/**************************************************************************/
/**
 * @brief This function sends the http/https request and flashes the new
 * firmware on the memory
 *
 * @param httpClient
 * @return false: Ota update was not successful
 */
/**************************************************************************/
bool OtaUpdate::_handleOTA(HttpClient *httpClient, const char *host, const char *path) {
    int err = 0;
    static char message[80];
    unsigned long mqttTimeStamp = millis();
    unsigned long mqttInterval = 5000;
    unsigned long durationTimeStamp = millis();
    if (_eth->localIP() != IPAddress(0, 0, 0, 0) && _eth->linkUp()) {
        // Sends HTTP-request
        httpClient->beginRequest();
        // Apache webserver requires these header informations
        err = httpClient->get(path);
        httpClient->sendHeader("Accept", "*/*");
        httpClient->sendHeader("Host", host);
        httpClient->sendBasicAuth(OTA_USER_NAME, OTA_PASSWORD); // send the username and password for authentication
        httpClient->endRequest();

        // If connection the server was not successfull
        if (err != 0) {
            LOG.error("[CL-FU-01] OTA Update Server Unavailable");
            httpClient->stop();
            return false;
        }

        // Check server response
        int statusCode = httpClient->responseStatusCode();
        LOG.info(String("HTTP-Request Status Code " + String(statusCode)));
        if (statusCode != 200) {
            LOG.error("[CL-FU-02] Download Failed");
            httpClient->stop();
            return false;
        }

        // Checks if size of content length header is right
        long length = httpClient->contentLength();
        if (length == HttpClient::kNoContentLengthHeader) {
            LOG.error("[CL-FU-02] Download Failed");
            LOG.info("Server didn't provide Content-length header");
            httpClient->stop();
            return false;
        }
        LOG.info(String("Server returned update file of size " + String(length) + " Byte"));

        // Check if enough storage capacity is left
        if (!InternalStorage.open(length)) {
            LOG.error("[CL-FU-02] Download Failed");
            LOG.info("Not enough space to store the update");
            httpClient->stop();
            return false;
        }

        // Start writing the downloaded firmware in storage
        byte b;
        while (length > 0) {
            if (!httpClient->readBytes(&b, 1))
                break;
            InternalStorage.write(b);
            length--;
            if (millis() - mqttTimeStamp > mqttInterval) {
                _mqttClient->loop();
                mqttTimeStamp = millis();
            }
        }
        InternalStorage.close();
        httpClient->stop();
        if (length > 0) {
            LOG.error("[CL-FU-02] Download Failed");
            LOG.info(String("Timeout downloading update file at Byte " + String(length)));
            return false;
        }

        // Reset update available notification
        _mqttClient->publish(_topic.c_str(), "");
        _mqttClient->loop();
        float time = (millis() - durationTimeStamp) / 1000;
        LOG.debug(String("Download took " + String(time) + "s"));
        LOG.info("Reboot...");
        _mqttClient->sendLastWillMessage();
        _mqttClient->disconnect();
        // Flash new firmware
        Serial.flush();
        InternalStorage.apply();
        return true;
    } else {
        return false;
        LOG.error("[CL-NC-01] Ethernet Network Connection Failed");
        LOG.error("[CL-FU-01] OTA Update Server Unavailable");
    }
}

/**************************************************************************/
/**
 * @brief This function initiates a http request to download a json file
 * which should contain all latest firmware information
 *
 * @param url
 * @return true
 * @return false
 */
/**************************************************************************/
bool OtaUpdate::requestLatestFirmware(const char *url) {
    // Check for network connection
    if (_eth->localIP() != IPAddress(0, 0, 0, 0) && _eth->linkUp()) {
        unsigned long debugTime = millis();
        String domain, path;
        unsigned short port;
        int err;
        String incomingMessage;

        splitUrl(url, domain, port, path);

        LOG.info("Begin download of latest firmware information ");
        _mqttClient->loop();

        httpClient = HttpClient(*_client, domain, port);
        LOG.info(String("HTTP-Request " + String(domain) + ":" + String(port) + "/" + path));

        // Sends HTTP-request
        httpClient.beginRequest();
        // Apache webserver requires these header informations
        err = httpClient.get(path);
        httpClient.sendHeader("Accept", "*/*");
        httpClient.sendHeader("Host", domain);
        httpClient.sendBasicAuth(OTA_USER_NAME, OTA_PASSWORD); // send the username and password for authentication

        httpClient.endRequest();

        // If connection the server was not successfull
        if (err != 0) {
            LOG.error("[CL-FU-01] OTA Update Server Unavailable");
            _mqttClient->publish(_topic.c_str(), "Server is unavailable, can't check for latest firmware information");
            httpClient.stop();
            return false;
        }

        // Check server response
        int statusCode = httpClient.responseStatusCode();
        LOG.info(String("HTTP-Request Status Code " + String(statusCode)));
        if (statusCode != 200) {
            LOG.error("[CL-FU-02] Download Failed");
            _mqttClient->publish(_topic.c_str(), "Server is unavailable, can't check for latest firmware information");
            httpClient.stop();
            return false;
        }
        incomingMessage = httpClient.responseBody();
        httpClient.stop();
        float time = (millis() - debugTime) / 1000;
        LOG.info(String("Server returned latest firmware information: " + incomingMessage));
        readJson(incomingMessage);
        LOG.debug(String("Download took " + String(time) + "s"));
        return true;
    } else {
        LOG.error("[CL-NC-01] Ethernet Network Connection Failed");
        LOG.error("[CL-FU-01] OTA Update Server Unavailable");
        return false;
    }
}

/**************************************************************************/
/**
 * @brief This function analyses the json file which contains all latest firmware information
 *
 * @param jsonMessage
 */
/**************************************************************************/
void OtaUpdate::readJson(String jsonMessage) {
    DynamicJsonDocument doc(500);
    DeserializationError error = deserializeJson(doc, jsonMessage);
    if (error) {
        LOG.error("[CL-FU-02] Download Failed");

        return;
    }
    if (doc.containsKey(PRODUCT_TYPE)) {
        if (doc[PRODUCT_TYPE].containsKey("version")) {
            String latestFwVersion = doc[PRODUCT_TYPE]["version"].as<const char *>();
            LOG.info(String("Current firmware version: " + String(FIRMWARE_VERSION)));
            LOG.info(String("Latest firmware version: " + latestFwVersion));

            bool newFwAvailable = compareFwVersion(FIRMWARE_VERSION, latestFwVersion.c_str());
            LOG.info(String("Is new firmware available: " + String(newFwAvailable)).c_str());

            if (newFwAvailable) {
                if (!_topic.isEmpty()) {
                    _mqttClient->publish(_topic.c_str(), latestFwVersion.c_str());
                }
                if (doc[PRODUCT_TYPE].containsKey("url")) {
                    newFirmwareUrl = doc[PRODUCT_TYPE]["url"].as<const char *>();
                }
            } else {
                if (!_topic.isEmpty()) {
                    _mqttClient->publish(_topic.c_str(), latestFwVersion.c_str());
                }
            }
        }
    } else {
        _mqttClient->publish(_topic.c_str(), "No update available");
        LOG.info("No update available");
    }
}

/**************************************************************************/
/**
 * @brief Compares old and new firmware version number and returns if new version is available
 *
 * @param oldFw
 * @param newFw
 * @return true: new firmware available
 * @return false no firmware available
 */
/**************************************************************************/
bool OtaUpdate::compareFwVersion(const char *oldFw, const char *newFw) {
    int oldFwMajor, oldFwMinor, oldFwPatch;
    sscanf(oldFw, "%i.%i.%i", &oldFwMajor, &oldFwMinor, &oldFwPatch);

    int newFwMajor, newFwMinor, newFwPatch;
    sscanf(newFw, "%i.%i.%i", &newFwMajor, &newFwMinor, &newFwPatch);
    if (newFwMajor > oldFwMajor)
        return true;
    else if (newFwMajor == oldFwMajor) {
        if (newFwMinor > oldFwMinor)
            return true;
        else if (newFwMinor == oldFwMinor) {
            if (newFwPatch > oldFwPatch)
                return true;
        }
    }

    return false;
}

/**************************************************************************/
/**
 * @brief This function returns true if a new firmware is available.
 *      This is just possible after request latest firmware json file
 *
 * @return true
 * @return false
 */
/**************************************************************************/
bool OtaUpdate::isFwAvailable() {
    return !newFirmwareUrl.isEmpty();
}

/**************************************************************************/
/**
 * @brief This function splits an url into three components domain, port, path.
 * If url contanis http port = 80, If url contains https port = 443
 *
 * @param url
 * @param domain
 * @param port
 * @param path
 */
/**************************************************************************/
void OtaUpdate::splitUrl(String url, String &domain, unsigned short &port, String &path) {
    if (url.startsWith("https://")) {
        port = HTTPS_PORT;
        url = url.substring(8); // remove "https://"
    } else if (url.startsWith("http://")) {
        port = HTTP_PORT;
        url = url.substring(7); // remove "http://"
    }

    // DNS und Pfad extrahieren
    int slashIndex = url.indexOf('/');
    domain = url.substring(0, slashIndex); // extract DNS
    path = url.substring(slashIndex);      // extract Pfad
}

/**************************************************************************/
/**
 * @brief This function sets the url which will be used for requesting the latest firmware json file
 *
 * @param fileUrl
 */
/**************************************************************************/
void OtaUpdate::setFwVersionFileUrl(const char *fileUrl) {
    fwVersionFileUrl = fileUrl;
}

void OtaUpdate::startOfficialUpdate() {
    if (!newFirmwareUrl.isEmpty()) {
        String domain, path;
        unsigned short port;
        splitUrl(newFirmwareUrl, domain, port, path);
        requestFirmware(domain.c_str(), port, path.c_str());
    } else {
        LOG.warning("No update information available. Check for updates first");
    }
}

/**************************************************************************/
/**
 * @brief This function triggers the http request for downloading the latest firmware json file
 *
 */
/**************************************************************************/
void OtaUpdate::startOfficialVersionRequest() {
    requestLatestFirmware(fwVersionFileUrl.c_str());
}

/**************************************************************************/
/**
 * @brief This function sets the topic under which the new firmware version will be published if one is available
 *
 * @param topic
 */
/**************************************************************************/
void OtaUpdate::setUpdateAvailableTopic(const char *topic) {
    _topic = topic;
}
