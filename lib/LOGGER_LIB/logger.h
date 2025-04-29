#ifndef LOGGER_H
#define LOGGER_H

#include "mqtt_client.h"
#include <Preferences.h>

#define MAX_PUBLISH_ON_CONNECT 30

enum LogLevel {
    NONE,
    ERROR,
    WARNING,
    INFO,
    DEBUG
};

#define DEFAULT_LOG_LEVEL LogLevel::ERROR

class Logger {
public:
    // Singleton-Pattern: Makes sure just one instance exists
    static Logger &getInstance() {
        static Logger instance;
        return instance;
    }

    void begin(MqttClient *mqttClient, String topic);
    void error(String message);
    void warning(String message);
    void info(String message);
    void debug(String message);
    void setLogLevel(LogLevel logLevel);

private:
    Logger();
    MqttClient *_mqttClient;
    String _topic;
    uint8_t _publishOnConnectCounter = 0;
    LogLevel _currentLogLevel;
    Preferences _preferences;
};

#define LOG Logger::getInstance()

#endif // LOGGER_H