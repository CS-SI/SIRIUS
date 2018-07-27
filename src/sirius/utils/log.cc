/**
 * Copyright (C) 2018 CS - Systemes d'Information (CS-SI)
 *
 * This file is part of Sirius
 *
 *     https://github.com/CS-SI/SIRIUS
 *
 * Sirius is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Sirius is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Sirius.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "sirius/utils/log.h"

#include <iostream>

#ifdef SIRIUS_ENABLE_LOGS
#include <vector>
#ifdef _WIN32
#include <spdlog/sinks/wincolor_sink.h>
#else
#include <spdlog/sinks/ansicolor_sink.h>
#endif  // _WIN32

namespace sirius {
namespace utils {

LoggerManager& LoggerManager::Instance() {
    static LoggerManager manager_;
    return manager_;
}

void LoggerManager::SetLogLevel(spdlog::level::level_enum level) {
    log_level_ = level;
    spdlog::set_level(level);
}

LoggerManager::Logger* LoggerManager::Get(const std::string& channel) {
    std::lock_guard<std::mutex> lock(loggers_mutex_);
    if (loggers_.count(channel)) {
        return loggers_[channel].get();
    }

    std::vector<spdlog::sink_ptr> sinks;
#ifdef _WIN32
    sinks.push_back(std::make_shared<spdlog::sinks::wincolor_stderr_sink_mt>());
#else
    sinks.push_back(
          std::make_shared<spdlog::sinks::ansicolor_stderr_sink_mt>());
#endif  // _WIN32

    auto logger =
          std::make_shared<spdlog::logger>(channel, begin(sinks), end(sinks));
    logger->set_level(log_level_);
    spdlog::register_logger(logger);
    loggers_[channel] = logger;

    return logger.get();
}

}  // namespace utils
}  // namespace sirius

#endif  // SIRIUS_ENABLE_LOGS

namespace sirius {
namespace utils {

void SetVerbosityLevel(const std::string& level) {
    if (level == "trace") {
        LOG_SET_LEVEL_ENUM(spdlog::level::trace);
    } else if (level == "debug") {
        LOG_SET_LEVEL_ENUM(spdlog::level::debug);
    } else if (level == "info") {
        LOG_SET_LEVEL_ENUM(spdlog::level::info);
    } else if (level == "warn") {
        LOG_SET_LEVEL_ENUM(spdlog::level::warn);
    } else if (level == "err") {
        LOG_SET_LEVEL_ENUM(spdlog::level::err);
    } else if (level == "critical") {
        LOG_SET_LEVEL_ENUM(spdlog::level::critical);
    } else if (level == "off") {
        LOG_SET_LEVEL_ENUM(spdlog::level::off);
    } else {
        std::cerr << "sirius: invalid log level" << std::endl;
    }
}

}  // namespace utils
}  // namespace sirius
