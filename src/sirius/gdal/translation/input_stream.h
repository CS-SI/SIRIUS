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

#ifndef SIRIUS_GDAL_TRANSLATION_INPUT_STREAM_H_
#define SIRIUS_GDAL_TRANSLATION_INPUT_STREAM_H_

#include <string>

#include "sirius/exception.h"
#include "sirius/image.h"
#include "sirius/types.h"

#include "sirius/gdal/error_code.h"
#include "sirius/gdal/i_input_stream.h"
#include "sirius/gdal/input_stream.h"
#include "sirius/gdal/wrapper.h"

#include "sirius/translation/parameters.h"

#include "sirius/utils/log.h"

namespace sirius {
namespace gdal {
namespace translation {

class InputStream : public IInputStream {
  public:
    InputStream(const std::string& image_path, const sirius::Size& block_size,
                bool,
                const sirius::translation::Parameters& translation_parameters);

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
    StreamBlock Read(std::error_code& ec) override;

    /**
     * \brief Indicate end of image
     * \return boolean if end is reached
     */
    bool IsEnded() const override { return is_ended_; }

  private:
    gdal::DatasetUPtr input_dataset_;
    sirius::Size block_size_{256, 256};
    sirius::Size block_margin_size_;
    PaddingType block_padding_type_;
    bool is_ended_ = false;
    int row_idx_ = 0;
    int col_idx_ = 0;
    float row_shift_;
    float col_shift_;
};

}  // namespace translation
}  // namespace gdal
}  // namespace sirius

#endif  // SIRIUS_GDAL_TRANSLATION_INPUT_STREAM_H_