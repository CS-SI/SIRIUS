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

#ifndef SIRIUS_GDAL_INPUT_STREAM_H_
#define SIRIUS_GDAL_INPUT_STREAM_H_

#include <system_error>

#include "sirius/image.h"
#include "sirius/types.h"

#include "sirius/gdal/stream_block.h"
#include "sirius/gdal/types.h"

namespace sirius {
namespace gdal {

/**
 * \brief Stream an image in block
 */
class InputStream {
  public:
    /**
     * \brief Instanciate an InputStreamer and set its block size
     * \param image_path path to the input image
     * \param block_size blocks size
     * \param block_margin_size block margin size
     * \param block_padding_type block padding type
     */
    InputStream(const std::string& image_path, const sirius::Size& block_size,
                const sirius::Size& block_margin_size,
                PaddingType block_padding_type);

    ~InputStream() = default;

    InputStream(const InputStream&) = delete;
    InputStream& operator=(const InputStream&) = delete;
    InputStream(InputStream&&) = delete;
    InputStream& operator=(InputStream&&) = delete;

    /**
     * \brief Get the size of the input file
     * \return input file size
     */
    sirius::Size Size() {
        return {input_dataset_->GetRasterYSize(),
                input_dataset_->GetRasterXSize()};
    }

    /**
     * \brief Read a block from the image
     * \param ec error code if operation failed
     * \return block read
     */
    StreamBlock Read(std::error_code& ec);

    /**
     * \brief Indicate end of image
     * \return boolean if end is reached
     */
    bool IsAtEnd() { return is_ended_; }

  private:
    gdal::DatasetUPtr input_dataset_;
    sirius::Size block_size_{256, 256};
    sirius::Size block_margin_size_;
    PaddingType block_padding_type_;
    bool is_ended_ = false;
    int row_idx_ = 0;
    int col_idx_ = 0;
};

}  // namespace gdal
}  // namespace sirius

#endif  // SIRIUS_GDAL_INPUT_STREAM_H_
