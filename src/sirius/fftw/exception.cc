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

#include "sirius/fftw/exception.h"

#include <sstream>

namespace sirius {
namespace fftw {

Exception::Exception(ErrorCode code) : sirius::Exception(""), err_code_(code) {
    std::stringstream ss;
    ss << "FFTW error - code " << static_cast<int>(err_code_);
    default_err_msg_ = ss.str();
}

const char* Exception::what() const noexcept {
    switch (err_code_) {
        default:
            return default_err_msg_.c_str();
    }
}

}  // namespace fftw
}  // namespace sirius
