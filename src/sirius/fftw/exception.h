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

#ifndef SIRIUS_FFTW_EXCEPTION_H_
#define SIRIUS_FFTW_EXCEPTION_H_

#include <string>

#include "sirius/exception.h"

namespace sirius {
namespace fftw {

/**
 * \brief FFTW error codes
 */
enum class ErrorCode {
    kSuccess = 0,             /**< success*/
    kMemoryAllocationFailed,  /**< memory allocation failed */
    kPlanCreationFailed,      /**< plan creation failed */
    kComplexAllocationFailed, /**< complex allocation failed */
    kRealAllocationFailed     /**< real allocation failed */
};

/**
 * \brief Exception wrapper for FFTW error
 */
class Exception : public sirius::Exception {
  public:
    Exception(ErrorCode code);

    Exception(const Exception&) = default;
    Exception& operator=(const Exception&) = default;
    Exception(Exception&&) = default;
    Exception& operator=(Exception&&) = default;

    const char* what() const noexcept override;

  private:
    ErrorCode err_code_;
    std::string default_err_msg_;
};

}  // namespace fftw
}  // namespace sirius

#endif  // SIRIUS_FFTW_EXCEPTION_H_
