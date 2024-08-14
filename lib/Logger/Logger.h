#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

class Logger
{
public:
    enum LogLevel
    {
        INFO,
        WARNING,
        ERROR
    };

    static void log(const String &message, LogLevel level = INFO);

private:
    static String getLogLevelString(LogLevel level);
};

#endif // LOGGER_H