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

#ifndef SIRIUS_GDAL_RESAMPLING_INPUT_STREAM_H_
#define SIRIUS_GDAL_RESAMPLING_INPUT_STREAM_H_

#include <string>

#include "sirius/image.h"
#include "sirius/types.h"

#include "sirius/gdal/i_input_stream.h"
#include "sirius/gdal/input_stream.h"

#include "sirius/resampling/parameters.h"

namespace sirius {
namespace gdal {
namespace resampling {

class InputStream : public IInputStream {
  public:
    InputStream(const std::string& image_path, const sirius::Size& block_size,
                bool allow_block_resizing,
                const sirius::resampling::Parameters& resampling_parameters);

    ~InputStream() = default;
    InputStream(const InputStream&) = delete;
    InputStream& operator=(const InputStream&) = delete;
    InputStream(InputStream&&) = default;
    InputStream& operator=(InputStream&&) = default;

    /**
     * \brief Read a block from the image
     * \param ec error code if operation failed
     * \return block read
     */
    StreamBlock Read(std::error_code& ec) override {
        return input_stream_.Read(ec);
    }

    /**
     * \brief Indicate end of image
     * \return boolean if end is reached
     */
    bool IsEnded() const override { return input_stream_.IsEnded(); }

  private:
    gdal::InputStream input_stream_;
};

}  // namespace resampling
}  // namespace gdal
}  // namespace sirius

#endif  // SIRIUS_GDAL_RESAMPLING_INPUT_STREAM_H_