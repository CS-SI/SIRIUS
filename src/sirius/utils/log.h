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

#ifndef SIRIUS_UTILS_LOG_H_
#define SIRIUS_UTILS_LOG_H_

#include <string>

#ifdef SIRIUS_ENABLE_LOGS

#include <map>
#include <memory>
#include <mutex>

#include <spdlog/spdlog.h>

namespace sirius {
namespace utils {

class LoggerManager {
  public:
    using Logger = spdlog::logger;
    using LoggerSPtr = std::shared_ptr<Logger>;

  public:
    static LoggerManager& Instance();

    void SetLogLevel(spdlog::level::level_enum level);
    Logger* Get(const std::string& channel);

  private:
    LoggerManager() = default;

  private:
    std::mutex loggers_mutex_;
};

}  // namespace utils
}  // namespace sirius

#define LOG_SET_LEVEL_ENUM(lvl_enum) \
    sirius::utils::LoggerManager::Instance().SetLogLevel(lvl_enum)
#define LOG_SET_LEVEL(lvl) LOG_SET_LEVEL_ENUM(spdlog::level::lvl)
#define LOG(channel, level, ...) \
    sirius::utils::LoggerManager::Instance().Get(channel)->level(__VA_ARGS__)

#else

#define LOG_SET_LEVEL_ENUM(lvl_enum)
#define LOG_SET_LEVEL(lvl)
#define LOG(channel, level, ...)

#endif  // SIRIUS_ENABLE_LOGS

namespace sirius {
namespace utils {

/**
 * \brief Set verbosity level from string
 * \param level verbosity level: trace,debug,info,warn,error,critical,off
 */
void SetVerbosityLevel(const std::string& level);

}  // namespace utils
}  // namespace sirius

#endif  // SIRIUS_UTILS_LOG_H_
