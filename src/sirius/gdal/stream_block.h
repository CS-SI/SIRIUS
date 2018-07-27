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

#ifndef SIRIUS_GDAL_STREAM_H_
#define SIRIUS_GDAL_STREAM_H_

#include "sirius/image.h"

namespace sirius {
namespace gdal {

/**
 * \brief Stream block
 */
struct StreamBlock {
    StreamBlock() = default;

    /**
     * \brief Instanciate a stream block from its block image and its position
     *        in the input image
     *
     * \param block_image image buffer of this block
     * \param row_idx row index of the top left corner in the input image
     * \param col_idx col index of the top left corner in the input image
     * \param padding required filter padding
     */
    StreamBlock(Image&& i_block_image, int i_row_idx, int i_col_idx,
                const Padding& i_padding)
        : buffer(std::move(i_block_image)),
          row_idx(i_row_idx),
          col_idx(i_col_idx),
          padding(i_padding),
          is_initialized(true) {}

    ~StreamBlock() = default;
    StreamBlock(const StreamBlock&) = default;
    StreamBlock& operator=(const StreamBlock&) = default;
    StreamBlock(StreamBlock&&) = default;
    StreamBlock& operator=(StreamBlock&&) = default;

    Image buffer{};
    int row_idx = 0;
    int col_idx = 0;
    Padding padding{};
    bool is_initialized = false;
};

}  // namespace gdal
}  // namespace sirius

#endif  // SIRIUS_GDAL_STREAM_H_
