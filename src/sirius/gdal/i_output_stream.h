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

#ifndef SIRIUS_I_GDAL_OUTPUT_STREAM_H_
#define SIRIUS_I_GDAL_OUTPUT_STREAM_H_

#include <system_error>

#include "sirius/types.h"

#include "sirius/gdal/stream_block.h"

namespace sirius {
namespace gdal {

class IOutputStream {
  public:
    virtual ~IOutputStream() = default;

    /**
     * \brief Read a block from the image
     * \param ec error code if operation failed
     * \return block read
     */
    virtual void Write(StreamBlock&& block, std::error_code& ec) = 0;
};

}  // namespace gdal
}  // namespace sirius

#endif  // SIRIUS_I_GDAL_OUTPUT_STREAM_H_