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

#ifndef SIRIUS_GDAL_EXCEPTION_H_
#define SIRIUS_GDAL_EXCEPTION_H_

#include <string>

#include <cpl_error.h>

#include <sirius/exception.h>

namespace sirius {
namespace gdal {

/**
 * \brief Exception wrapper for GDAL error
 */
class Exception : public sirius::Exception {
  public:
    /**
     * \brief Automatically extract GDAL error level and code from last GDAL
     *        error
     */
    Exception();

    Exception(CPLErr level, CPLErrorNum code);

    Exception(const Exception&) = default;
    Exception& operator=(const Exception&) = default;
    Exception(Exception&&) = default;
    Exception& operator=(Exception&&) = default;

    const char* what() const noexcept override;

  private:
    ::CPLErr err_level_;
    ::CPLErrorNum err_code_;
    std::string default_err_msg_;
};

}  // namespace gdal
}  // namespace sirius

#endif  // SIRIUS_GDAL_EXCEPTION_H_
