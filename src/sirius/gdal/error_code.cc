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

#include "sirius/gdal/error_code.h"

namespace sirius {
namespace gdal {

class ErrorCategory : public std::error_category {
    const char* name() const noexcept override;
    std::string message(int ev) const override;
};

const char* ErrorCategory::name() const noexcept { return "gdal"; }

std::string ErrorCategory::message(int ev) const {
    switch (static_cast<ErrorCode>(ev)) {
        case ErrorCode::kNone:
            return "No error";
        case ErrorCode::kAppDefined:
            return "Application defined error";
        case ErrorCode::kOutOfMemory:
            return "Out of memory error";
        case ErrorCode::kFileIO:
            return "File I/O error";
        case ErrorCode::kOpenFailed:
            return "Open failed";
        case ErrorCode::kIllegalArg:
            return "Illegal argument";
        case ErrorCode::kNotSupported:
            return "Not supported";
        case ErrorCode::kAssertionFailed:
            return "Assertion failed";
        case ErrorCode::kNoWriteAccess:
            return "No write access";
        case ErrorCode::kUserInterrupt:
            return "User interrupted";
        case ErrorCode::kObjectNull:
            return "NULL object";
        case ErrorCode::kHttpResponse:
            return "HTTP response";
        case ErrorCode::kAWSBucketNotFound:
            return "AWS bucket not found";
        case ErrorCode::kAWSObjectNotFound:
            return "AWS object not found";
        case ErrorCode::kAWSAccessDenied:
            return "AWS access denied";
        case ErrorCode::kAWSInvalidCredentials:
            return "AWS invalid credentials";
        case ErrorCode::kAWSSignatureDoesNotMatch:
            return "AWS signature does not match";
        default:
            return "unknown error";
    }
}

ErrorCategory& GetErrorCategory() {
    static ErrorCategory category;
    return category;
}

std::error_code make_error_code(::CPLErrorNum errc) {
    return {static_cast<int>(errc), GetErrorCategory()};
}

}  // namespace gdal
}  // namespace sirius
