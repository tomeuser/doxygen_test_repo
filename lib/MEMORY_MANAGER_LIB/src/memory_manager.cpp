/**************************************************************************/
/**
 * @file memory_manager.cpp
 * @author Tom Meuser (t.meuser@innovaze-media.com)
 * @brief This file defines all methodes of class MemoryManager which were
 * declared in memory_manager.h
 * @version 0.1
 * @date 2024-04-15
 *
 * @copyright Copyright (c) 2024, Adunas GmbH
 *            All rights reserved.
 *            Unauthorized copying of this file, via any medium is strictly
 *            prohibited Proprietary and confidential *
 */
/**************************************************************************/
#include "memory_manager.h"

/**************************************************************************/
/**
 * @brief This function reads the device configuration from permanent memory. If
 * there's no configuration available this function reqests it from an
 * webserver. If the webserver can't provide the config file, this function
 * applys default values until a new file request will happen after reboot
 */
/**************************************************************************/
void MemoryManager::begin() {
    // Check if config is available on memory
    _readMemory();
}

/**************************************************************************/
/**
 * @brief This function reads the configuration data out of the memory. If there
 * are no data available it returns default values
 *
 */
/**************************************************************************/
void MemoryManager::_readMemory() {
    _preferences.begin(MEMORY_NAMESPACE, true); // read mode

    _macAddress = _preferences.getString("mac_address", readMacAdress());

    // Network settings
    _dhcp = _preferences.getBool("dhcp", DEFAULT_DHCP);
    _localIp = ipFromString(_preferences.getString("local_ip", DEFAULT_LOCAL_IP).c_str());
    _subnetMask = ipFromString(_preferences.getString("subnet_mask", DEFAULT_SUBNET_MASK).c_str());
    _gateway = ipFromString(_preferences.getString("gateway", DEFAULT_GATEWAY).c_str());
    _dns = ipFromString(_preferences.getString("dns", DEFAULT_DNS).c_str());

    // Ota settings
    _otaVersionFile = _preferences.getString("jsonFile", OTA_VERSION_FILE);

    _preferences.end();
    printAll();
}

/**************************************************************************/
/**
 * @brief This function analyses a JSON object and and writes its data into the
 * permanent memory
 *
 * @param doc JSON object
 */
/**************************************************************************/
void MemoryManager::updateMemory(DynamicJsonDocument &doc) {
    _preferences.begin(MEMORY_NAMESPACE, false); // write mode

    // Network settings
    if (doc.containsKey("dhcp")) {
        _dhcp = doc["dhcp"].as<bool>();
        _preferences.putBool("dhcp", doc["dhcp"].as<bool>());
    }

    if (doc.containsKey("ip_address")) {
        _localIp = ipFromString(doc["ip_address"].as<const char *>());
        _preferences.putString("local_ip", doc["ip_address"].as<const char *>());
    }

    if (doc.containsKey("subnet_mask")) {
        _subnetMask = ipFromString(doc["subnet_mask"].as<const char *>());
        _preferences.putString("subnet_mask", doc["subnet_mask"].as<const char *>());
    }

    if (doc.containsKey("gateway")) {
        _gateway = ipFromString(doc["gateway"].as<const char *>());
        _preferences.putString("gateway", doc["gateway"].as<const char *>());
    }

    if (doc.containsKey("dns")) {
        _dns = ipFromString(doc["dns"].as<const char *>());
        _preferences.putString("dns", doc["dns"].as<const char *>());
    }

    // Ota settings
    if (doc.containsKey("ota")) {
        if (doc["ota"].containsKey("jsonFile")) {
            _otaVersionFile = doc["ota"]["jsonFile"].as<const char *>();
            _preferences.putString("jsonFile", doc["ota"]["jsonFile"].as<const char *>());
        }
    }
    _preferences.end();

    if (doc.containsKey("reset")) {
        reset();
        ESP.restart();
    }

    if (doc.containsKey("reboot")) {
        LOG.info("Reboot...");
        _mqttClient->sendLastWillMessage();
        _mqttClient->disconnect();
        ESP.restart();
    }
}

/**************************************************************************/
/**
 * @brief This function clears the values stored in permanent memory.
 *
 */
/**************************************************************************/
void MemoryManager::reset() {
    _preferences.begin(MEMORY_NAMESPACE, false);
    _preferences.clear();
    _preferences.end();
}

/**************************************************************************/
/**
 * @brief This function print all values stored in permanent memory
 *
 */
/**************************************************************************/
void MemoryManager::printAll() {

    Serial.println("Device Settings: ");
    Serial.println();

    Serial.print("Dhcp: ");
    Serial.println(_dhcp);

    if (!_dhcp) {
        Serial.print("Local ip: ");
        Serial.println(_localIp);

        Serial.print("Subnet mask: ");
        Serial.println(_subnetMask);

        Serial.print("Gateway: ");
        Serial.println(_gateway);

        Serial.print("DNS: ");
        Serial.println(_dns);
    }

    Serial.print("Ota version file: ");
    Serial.println(_otaVersionFile);

    Serial.println();
}

/**************************************************************************/
/**
 * @brief This function converts an ip address provided as a string into an
 * IPAddress object
 *
 * @param ip Ip address (const char*)
 * @return Ip address (IPAddress)
 */
/**************************************************************************/
IPAddress MemoryManager::ipFromString(const char *ip) {
    if (ip == "")
        return IPAddress(0, 0, 0, 0);
    uint8_t ipObj[4];
    sscanf(ip, "%u.%u.%u.%u", &ipObj[0], &ipObj[1], &ipObj[2], &ipObj[3]);
    return IPAddress(ipObj[0], ipObj[1], ipObj[2], ipObj[3]);
}

/**************************************************************************/
/**
 * @brief This function reads the unique mac address from the chip and converts
 * it into a string
 *
 * @return const char* mac address
 */
/**************************************************************************/
const char *MemoryManager::readMacAdress() {
    static char macChar[13];
    byte macByte[6];
    _ethernetInterface->macAddress(macByte);
    for (int i = 0; i < 6; i++) {
        sprintf(&macChar[i * 2], "%02X", macByte[i]);
    }
    return macChar;
}

String MemoryManager::getMacAddress() {
    return _macAddress;
}

// Getter
bool MemoryManager::getDhcp() { return _dhcp; }

IPAddress MemoryManager::getLocalIp() { return _localIp; }

IPAddress MemoryManager::getSubnetMask() { return _subnetMask; }

IPAddress MemoryManager::getGateway() { return _gateway; }

IPAddress MemoryManager::getDns() { return _dns; }

String MemoryManager::getOtaVersionFile() { return _otaVersionFile; }
