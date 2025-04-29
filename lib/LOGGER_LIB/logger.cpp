#include "logger.h"

Logger::Logger() {
    _preferences.begin(MEMORY_NAMESPACE, true);
    _currentLogLevel = LogLevel(_preferences.getInt("logLevel", (DEFAULT_LOG_LEVEL)));
    _preferences.end();
}

void Logger::begin(MqttClient *mqttClient, String topic) {
    _mqttClient = mqttClient;
    _topic = topic;
    _mqttClient->publishOnConnect(String(_topic + "/logging_level"), String(_currentLogLevel), true);
}

void Logger::error(String message) {
    if (_currentLogLevel >= LogLevel::ERROR) {
        String log = String("[E] " + message);
        Serial.println(log);

        if (_mqttClient->connected()) {
            _mqttClient->publish(String(_topic + "/error").c_str(), log.c_str());
        } else if (_publishOnConnectCounter < MAX_PUBLISH_ON_CONNECT) {
            _mqttClient->publishOnConnect(String(_topic + "/error"), log, false);
            _publishOnConnectCounter++;
        }
    }
}

void Logger::warning(String message) {
    if (_currentLogLevel >= LogLevel::WARNING) {
        String log = String("[W] " + message);
        Serial.println(log);

        if (_mqttClient->connected()) {
            _mqttClient->publish(String(_topic + "/logging").c_str(), log.c_str());
        } else if (_publishOnConnectCounter < MAX_PUBLISH_ON_CONNECT) {
            _mqttClient->publishOnConnect(String(_topic + "/logging"), log, false);
            _publishOnConnectCounter++;
        }
    }
}

void Logger::info(String message) {
    if (_currentLogLevel >= LogLevel::INFO) {
        String log = String("[I] " + message);
        Serial.println(log);

        if (_mqttClient->connected()) {
            _mqttClient->publish(String(_topic + "/logging").c_str(), log.c_str());
        } else if (_publishOnConnectCounter < MAX_PUBLISH_ON_CONNECT) {
            _mqttClient->publishOnConnect(String(_topic + "/logging"), log, false);
            _publishOnConnectCounter++;
        }
    }
}

void Logger::debug(String message) {
    if (_currentLogLevel >= LogLevel::DEBUG) {
        String log = String("[D] " + message);
        Serial.println(log);

        if (_mqttClient->connected()) {
            _mqttClient->publish(String(_topic + "/logging").c_str(), log.c_str());
        } else if (_publishOnConnectCounter < MAX_PUBLISH_ON_CONNECT) {
            _mqttClient->publishOnConnect(String(_topic + "/logging"), log, false);
            _publishOnConnectCounter++;
        }
    }
}

void Logger::setLogLevel(LogLevel logLevel) {
    _currentLogLevel = logLevel;
    _preferences.begin(MEMORY_NAMESPACE, false);
    _preferences.putInt("logLevel", logLevel);
    _preferences.end();
    _mqttClient->publish(String(_topic + "/logging_level").c_str(), String(_currentLogLevel).c_str(), true);
}
