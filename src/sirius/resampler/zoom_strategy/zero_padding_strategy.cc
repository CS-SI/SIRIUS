/**
 * Copyright (C) 2018 CS - Systeme d'Information (CS-SI)
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

#include "sirius/resampler/zoom_strategy/zero_padding_strategy.h"

#include <algorithm>

#include "sirius/fftw/exception.h"
#include "sirius/fftw/fftw.h"
#include "sirius/fftw/types.h"
#include "sirius/fftw/wrapper.h"

#include "sirius/exception.h"

#include "sirius/utils/gsl.h"
#include "sirius/utils/log.h"

namespace sirius {
namespace resampler {

Image ZeroPaddingZoomStrategy::Zoom(int zoom, const Image& padded_image,
                                    const Filter& filter) const {
    // 1) FFT image
    LOG("zero_padding_zoom", trace, "compute image FFT {}x{}",
        padded_image.size.row, padded_image.size.col);
    auto image_fft = fftw::FFT(padded_image);

    // 2) zoom FFT
    LOG("zero_padding_zoom", trace, "zero pad FFT");
    auto zoomed_fft = ZeroPadFFT(zoom, padded_image, std::move(image_fft));

    Size zoomed_size{padded_image.size.row * zoom,
                     padded_image.size.col * zoom};

    if (filter.IsLoaded()) {
        // 3) Filter zoomed FFT
        LOG("zero_padding_zoom", trace, "apply filter");
        zoomed_fft = filter.Process(zoomed_size, std::move(zoomed_fft));
    }

    // 4) IFFT zoomed FFT
    LOG("zero_padding_zoom", trace, "compute image IFFT");
    auto zoomed_image = fftw::IFFT(zoomed_size, std::move(zoomed_fft));

    // 5) Normalize zoomed image
    LOG("zero_padding_zoom", trace, "normalize image");
    int pixel_count = padded_image.CellCount();
    std::for_each(zoomed_image.data.begin(), zoomed_image.data.end(),
                  [pixel_count](double& pixel) { pixel /= pixel_count; });
    return zoomed_image;
}

fftw::ComplexUPtr ZeroPaddingZoomStrategy::ZeroPadFFT(
      int zoom, const Image& image, fftw::ComplexUPtr image_fft) const {
    if (zoom <= 1) {
        // nothing to pad: 1:1 zoom
        return image_fft;
    }

    int image_row_count = image.size.row;
    int image_col_count = image.size.col;
    int half_row_count = std::ceil(image_row_count / 2.0);

    int fft_row_count = image_row_count;
    int fft_col_count = (image_col_count / 2) + 1;

    int zoomed_row_count = image_row_count * zoom;
    int zoomed_col_count = image_col_count * zoom;
    int fft_zoomed_row_count = zoomed_row_count;
    int fft_zoomed_col_count = zoomed_col_count / 2 + 1;

    Size zoomed_fft_size(fft_zoomed_row_count, fft_zoomed_col_count);
    auto zoomed_fft = fftw::CreateComplex(zoomed_fft_size);

    // zero padding zoom
    // 1) fill result with 0 (initialized in fftw::CreateComplex)
    // 2) split image_fft in two blocks: (0, half_row_count, 0, fft_col_count)
    //   and (half_row_count, fft_row_count, 0, fft_col_count)
    //   - copy first block in top-left corner of zoomed_fft
    //   - copy second block in bottom left corner of zoomed_fft

    int pixel_index = 0;
    int zoomed_row = 0;
    int zoomed_col = 0;
    int zoomed_pixel_index = 0;

    auto image_fft_span = utils::MakeSmartPtrArraySpan(image_fft, image.size);
    auto zoomed_fft_span =
          utils::MakeSmartPtrArraySpan(zoomed_fft, zoomed_fft_size);
    for (int row = 0; row < fft_row_count; ++row) {
        for (int col = 0; col < fft_col_count; ++col) {
            pixel_index = row * fft_col_count + col;

            if (row < half_row_count) {
                zoomed_row = row;
            } else {
                zoomed_row = zoomed_row_count - (fft_row_count - row);
            }
            zoomed_col = col;
            zoomed_pixel_index = zoomed_row * fft_zoomed_col_count + zoomed_col;

            // copy complex number from image_fft to zoomed_fft
            zoomed_fft_span[zoomed_pixel_index][0] =
                  image_fft_span[pixel_index][0];
            zoomed_fft_span[zoomed_pixel_index][1] =
                  image_fft_span[pixel_index][1];
        }
    }

    return zoomed_fft;
}

}  // namespace resampler
}  // namespace sirius
