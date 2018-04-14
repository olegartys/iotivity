//
// Created by olegartys on 08.01.17.
//

#ifndef SHOUSESERVERCOMPONENT_ILOG_H
#define SHOUSESERVERCOMPONENT_ILOG_H

#include <string>

#include "spdlog/include/spdlog/spdlog.h"

struct LogParamsHolder {
    static bool& LogPrintFile() {
        static bool logPrintFile;
        return logPrintFile;
    }

    static std::string& LogPrintFileName() { 
        static std::string logPrintFileName;
        return logPrintFileName;
    }
};

class Log {
public:
    Log() = delete;

    static void setOutputFile(const std::string& path) {
        LogParamsHolder::LogPrintFile() = true;
        LogParamsHolder::LogPrintFileName() = path;
    }

    static void setLevel(spdlog::level::level_enum level) {
        spdlog::set_level(level);
    }

    template <typename T>
    static void info(const std::string& TAG, const T& val);

    template <typename ...Args>
    static void info(const std::string& TAG, const char* fmt, const Args&... args);

    template <typename T>
    static void debug(const std::string& TAG, const T& val);

    template <typename ...Args>
    static void debug(const std::string& TAG, const char* fmt, const Args&... args);

    template <typename T>
    static void warn(const std::string& TAG, const T& val);

    template <typename ...Args>
    static void warn(const std::string& TAG, const char* fmt, const Args&... args);

    template <typename T>
    static void error(const std::string& TAG, const T& val);

    template <typename ...Args>
    static void error(const std::string& TAG, const char* fmt, const Args&... args);

    template <typename T>
    static void critical(const std::string& TAG, const T& val);

    template <typename ...Args>
    static void critical(const std::string& TAG, const char* fmt, const Args&... args);

};

#include "LogImpl.h"

#endif //SHOUSESERVERCOMPONENT_ILOG_H
