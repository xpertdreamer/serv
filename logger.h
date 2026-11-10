#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <iostream>
#include <chrono>
#include <mutex>
#include <format>

enum class LEVEL {
    INFO = 0,
    ERROR,
};

inline const char*
lvl_to_string(LEVEL lvl) {
    switch (lvl) {
        case LEVEL::INFO: return "INFO";
        case LEVEL::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

class Logger {
    private:
        std::mutex mutex;
        std::ofstream file;

        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

    public:
        Logger() = default;

        ~Logger() {
            if (file.is_open()) {
                file.close();
            }
        }

        inline bool
        open(const std::string& path) {
            std::lock_guard l(mutex);
            if (!file.is_open()) file.open(path, std::ios::app);
            return file ? true : false;
        }

        inline void
        fmt(LEVEL level, const std::string& msg) {
            std::lock_guard l(mutex);
            auto now = std::chrono::system_clock::now();
            if (file.is_open()) {
                file << std::format("{:%Y-%m-%d %H:%M:%S}", now) << "\t" <<  lvl_to_string(level) << "\t" << msg << '\n';
                file.flush();
            }
        }

        static Logger& instance() {
            static Logger l;
            return l;
        }
};

inline void
fexcept(LEVEL level, const std::string& msg) {
    Logger::instance().fmt(level, msg);
}

#endif
