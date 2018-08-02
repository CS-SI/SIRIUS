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

#ifndef SIRIUS_GDAL_ERROR_CODE_H_
#define SIRIUS_GDAL_ERROR_CODE_H_

#include <system_error>

#include <cpl_error.h>

// This implementation is inspired by
//   https://akrzemi1.wordpress.com/2017/07/12/your-own-error-code/

namespace sirius {
namespace gdal {

/**
 * \brief Enum of gdal error codes
 */
enum class ErrorCode {
    kNone = CPLE_None,
    kAppDefined = CPLE_AppDefined,
    kOutOfMemory = CPLE_OutOfMemory,
    kFileIO = CPLE_FileIO,
    kOpenFailed = CPLE_OpenFailed,
    kIllegalArg = CPLE_IllegalArg,
    kNotSupported = CPLE_NotSupported,
    kAssertionFailed = CPLE_AssertionFailed,
    kNoWriteAccess = CPLE_NoWriteAccess,
    kUserInterrupt = CPLE_UserInterrupt,
    kObjectNull = CPLE_ObjectNull,
    kHttpResponse = CPLE_HttpResponse,
    kAWSBucketNotFound = CPLE_AWSBucketNotFound,
    kAWSObjectNotFound = CPLE_AWSObjectNotFound,
    kAWSAccessDenied = CPLE_AWSAccessDenied,
    kAWSInvalidCredentials = CPLE_AWSInvalidCredentials,
    kAWSSignatureDoesNotMatch = CPLE_AWSSignatureDoesNotMatch,
};

/**
 * \brief Make an error code from ::CPLErrorNum enum
 *
 * \param errc error code
 *
 * \return Generated error code
 */
std::error_code make_error_code(::CPLErrorNum errc);

}  // namespace gdal
}  // namespace sirius

namespace std {

template <>
struct is_error_code_enum<sirius::gdal::ErrorCode> : std::true_type {};

}  // namespace std

#endif  // SIRIUS_GDAL_ERROR_CODE_H_
