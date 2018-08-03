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

#ifndef SIRIUS_RESAMPLER_FREQUENCY_RESAMPLER_TXX_
#define SIRIUS_RESAMPLER_FREQUENCY_RESAMPLER_TXX_

#include "sirius/resampler/frequency_resampler.h"

#include <algorithm>

#include "sirius/exception.h"

#include "sirius/fftw/types.h"
#include "sirius/fftw/wrapper.h"

#include "sirius/utils/log.h"

namespace sirius {
namespace resampler {

template <template <class> class ImageDecompositionPolicy, class ZoomStrategy>
Image FrequencyResampler<ImageDecompositionPolicy, ZoomStrategy>::Compute(
      const ZoomRatio& zoom_ratio, const Image& input_image,
      const Padding& image_padding, const Filter& filter) const {
    LOG("frequency_resampler", trace, "compute {}/{} zoom of the image",
        zoom_ratio.input_resolution(), zoom_ratio.output_resolution());

    // basic checks
    if (filter.IsLoaded() && !filter.CanBeApplied(zoom_ratio)) {
        LOG("frequency_resampler", error,
            "cannot apply this filter on this zoom ratio");
        throw Exception("cannot apply this filter on this zoom ratio");
    }

    LOG("frequency_resampler", trace, "pad image");
    auto padded_image = input_image.CreatePaddedImage(image_padding);

    LOG("frequency_resampler", trace, "decompose and zoom image");
    // method inherited from ImageDecompositionPolicy
    Image result_image = this->DecomposeAndZoom(zoom_ratio.input_resolution(),
                                                padded_image, filter);

    LOG("frequency_resampler", trace, "unpad zoomed image");
    auto result = UnpadImage(zoom_ratio, input_image, result_image,
                             image_padding, filter);

    if (zoom_ratio.IsRealZoom()) {
        result = DecimateImage(result, zoom_ratio);
    }

    return result;
}

template <template <class> class ImageDecompositionPolicy, class ZoomStrategy>
Image FrequencyResampler<ImageDecompositionPolicy, ZoomStrategy>::UnpadImage(
      const ZoomRatio& zoom_ratio, const Image& original_image,
      const Image& zoomed_image, const Padding& padding,
      const Filter& filter) const {
    auto input_size = original_image.size;

    auto filter_padding_size = filter.padding_size();

    if (padding.top == 0) {
        // input is already padded top for filter so substract 1 filter row
        // padding
        input_size.row -= filter_padding_size.row;
    }
    if (padding.bottom == 0) {
        // input is already padded bottom for filter so substract 1 filter row
        // padding
        input_size.row -= filter_padding_size.row;
    }
    if (padding.left == 0) {
        // input is already padded left for filter so substract 1 filter col
        // padding
        input_size.col -= filter_padding_size.col;
    }
    if (padding.right == 0) {
        // input is already padded right for filter so substract 1 filter col
        // padding
        input_size.col -= filter_padding_size.col;
    }

    // expected result size
    auto result_size = input_size * zoom_ratio.input_resolution();

    Image result(result_size);

    int top_filter_margin = filter_padding_size.row;
    int left_filter_margin = filter_padding_size.col;

    int zoomed_col_length = input_size.col * zoom_ratio.input_resolution();
    int zoomed_left_padding_size =
          left_filter_margin * zoom_ratio.input_resolution();

    // remove padding from processed image
    int begin_row_data = top_filter_margin * zoom_ratio.input_resolution();
    int end_row_data =
          (input_size.row + top_filter_margin) * zoom_ratio.input_resolution();
    for (int row = begin_row_data; row < end_row_data; ++row) {
        // data starts after left padding and col length is zoomed_col_length
        auto data_row_begin_it = zoomed_image.data.cbegin() +
                                 (row * zoomed_image.size.col) +
                                 zoomed_left_padding_size;
        auto data_row_end_it = data_row_begin_it + zoomed_col_length;

        auto result_row_begin_it =
              result.data.begin() + (row - begin_row_data) * result.size.col;

        std::copy(data_row_begin_it, data_row_end_it, result_row_begin_it);
    }

    return result;
}

template <template <class> class ImageDecompositionPolicy, class ZoomStrategy>
Image FrequencyResampler<ImageDecompositionPolicy, ZoomStrategy>::DecimateImage(
      const Image& zoomed_image, const ZoomRatio& zoom_ratio) const {
    LOG("frequency_resampler", trace, "decimate zoomed image by {}",
        zoom_ratio.output_resolution());

    Image decimated_image(
          {static_cast<int>(std::ceil(
                 zoomed_image.size.row /
                 static_cast<double>(zoom_ratio.output_resolution()))),
           static_cast<int>(std::ceil(
                 zoomed_image.size.col /
                 static_cast<double>(zoom_ratio.output_resolution())))});

    int result_row = 0;
    for (int i = 0; i < zoomed_image.size.row;
         i += zoom_ratio.output_resolution()) {
        int result_col = 0;
        for (int j = 0; j < zoomed_image.size.col;
             j += zoom_ratio.output_resolution()) {
            decimated_image.Set(result_row, result_col, zoomed_image.Get(i, j));
            ++result_col;
        }
        ++result_row;
    }

    return decimated_image;
}

}  // namespace resampler
}  // namespace sirius

#endif  // SIRIUS_RESAMPLER_FREQUENCY_RESAMPLER_TXX_
