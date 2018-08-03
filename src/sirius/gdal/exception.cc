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

#include "sirius/gdal/exception.h"

#include <sstream>

namespace sirius {
namespace gdal {

Exception::Exception()
    : Exception(::CPLGetLastErrorType(), ::CPLGetLastErrorNo()) {}

Exception::Exception(::CPLErr level, ::CPLErrorNum code)
    : sirius::Exception(""), err_level_(level), err_code_(code) {
    std::stringstream ss;
    ss << "GDAL error level " << err_level_ << " : code " << err_code_;
    default_err_msg_ = ss.str();
}

const char* Exception::what() const noexcept {
    switch (err_code_) {
        case CPLE_None:
            return "No error";
        case CPLE_AppDefined:
            return "Application defined error";
        case CPLE_OutOfMemory:
            return "Out of memory error";
        case CPLE_FileIO:
            return "File I/O error";
        case CPLE_OpenFailed:
            return "Open failed";
        case CPLE_IllegalArg:
            return "Illegal argument";
        case CPLE_NotSupported:
            return "Not supported";
        case CPLE_AssertionFailed:
            return "Assertion failed";
        case CPLE_NoWriteAccess:
            return "No write access";
        case CPLE_UserInterrupt:
            return "User interrupted";
        case CPLE_ObjectNull:
            return "NULL object";
        case CPLE_HttpResponse:
            return "HTTP response";
        case CPLE_AWSBucketNotFound:
            return "AWS bucket not found";
        case CPLE_AWSObjectNotFound:
            return "AWS object not found";
        case CPLE_AWSAccessDenied:
            return "AWS access denied";
        case CPLE_AWSInvalidCredentials:
            return "AWS invalid credentials";
        case CPLE_AWSSignatureDoesNotMatch:
            return "AWS signature does not match";
        default:
            return default_err_msg_.c_str();
    }
}

}  // namespace gdal
}  // namespace sirius
