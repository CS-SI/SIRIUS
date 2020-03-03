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

#include "sirius/gdal/resampling/input_stream.h"

#include "sirius/utils/numeric.h"

namespace sirius {
namespace gdal {
namespace resampling {

InputStream::InputStream(
      const std::string& image_path, const Size& block_size,
      bool allow_block_resizing,
      const sirius::resampling::Parameters& resampling_parameters) {
    auto stream_block_size = block_size;

    Size filter_padding_size = {0, 0};
    PaddingType padding_type = PaddingType::kMirrorPadding;

    if (resampling_parameters.filter) {
        filter_padding_size = resampling_parameters.filter->padding_size();
        padding_type = resampling_parameters.filter->padding_type();
    }

    // improve stream_block_size if requested or required
    if (!resampling_parameters.ratio.IsRealZoom()) {
        if (allow_block_resizing) {
            stream_block_size = utils::GenerateDyadicSize(
                  stream_block_size,
                  resampling_parameters.ratio.input_resolution(),
                  filter_padding_size);
            LOG("resampled_input_stream", warn,
                "stream block resized to dyadic size: {}x{}",
                stream_block_size.row, stream_block_size.col);
        }
    } else {
        // real zoom needs specific block size (row and col should be
        // multiple
        // of input resolution and output resolution)
        stream_block_size = utils::GenerateZoomCompliantSize(
              stream_block_size, resampling_parameters.ratio);
        LOG("resampled_input_stream", warn,
            "stream block resized to comply with zoom: {}x{}",
            stream_block_size.row, stream_block_size.col);
    }
    input_stream_ = gdal::InputStream(image_path, stream_block_size,
                                      filter_padding_size, padding_type);
}

}  // namespace resampling
}  // namespace gdal
}  // namespace sirius