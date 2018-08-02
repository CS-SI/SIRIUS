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

#ifndef SIRIUS_UTILS_CONCURRENT_ERROR_CODE_H_
#define SIRIUS_UTILS_CONCURRENT_ERROR_CODE_H_

#include <system_error>

// This implementation is inspired by
//   https://akrzemi1.wordpress.com/2017/07/12/your-own-error-code/

namespace sirius {
namespace utils {

/**
 * \brief Enum of concurrent queue error codes
 */
enum class ConcurrentQueueErrorCode {
    kSuccess = 0,     /**< success */
    kQueueIsNotActive /**< queue is not active error */
};

/**
 * \brief Make an error code from ConcurrentQueueErrorCode enum
 *
 * \param errc error code
 *
 * \return Generated error code
 */
std::error_code make_error_code(ConcurrentQueueErrorCode errc);

}  // namespace utils
}  // namespace sirius

namespace std {

template <>
struct is_error_code_enum<sirius::utils::ConcurrentQueueErrorCode>
      : std::true_type {};

}  // namespace std

#endif  // SIRIUS_UTILS_CONCURRENT_ERROR_CODE_H_
