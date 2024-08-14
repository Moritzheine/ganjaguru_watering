#include "Logger.h"

void Logger::log(const String &message, LogLevel level)
{
    String timestamp = String(millis());
    String logMessage = "[" + timestamp + "] " + getLogLevelString(level) + ": " + message;
    Serial.println(logMessage);
}

String Logger::getLogLevelString(LogLevel level)
{
    switch (level)
    {
    case INFO:
        return "INFO";
    case WARNING:
        return "WARNING";
    case ERROR:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}