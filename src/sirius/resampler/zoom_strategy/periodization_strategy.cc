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

#include "sirius/resampler/zoom_strategy/periodization_strategy.h"

#include <algorithm>

#include "sirius/exception.h"

#include "sirius/fftw/exception.h"
#include "sirius/fftw/types.h"
#include "sirius/fftw/wrapper.h"

#include "sirius/utils/gsl.h"
#include "sirius/utils/log.h"

namespace sirius {
namespace resampler {

Image PeriodizationZoomStrategy::Zoom(int zoom, const Image& padded_image,
                                      const Filter& filter) const {
    // 1) FFT image
    LOG("periodization_zoom", trace, "compute image FFT");
    auto fft_image = fftw::FFT(padded_image);

    fftw::ComplexUPtr zoomed_fft;
    // 2) zoom FFT
    LOG("periodization_zoom", trace, "periodize FFT");
    zoomed_fft = PeriodizeFFT(zoom, padded_image, std::move(fft_image));

    Size zoomed_size{padded_image.size.row * zoom,
                     padded_image.size.col * zoom};

    if (filter.IsLoaded()) {
        // 3) Filter zoomed FFT
        LOG("periodization_zoom", trace, "apply filter");
        zoomed_fft = filter.Process(zoomed_size, std::move(zoomed_fft));
    }

    // 4) IFFT zoomed FFT
    LOG("periodization_zoom", trace, "compute image IFFT");
    auto zoomed_image = fftw::IFFT(zoomed_size, std::move(zoomed_fft));

    // 5) Normalize zoomed image
    LOG("periodization_zoom", trace, "normalize image");
    int pixel_count = padded_image.CellCount();
    std::for_each(zoomed_image.data.begin(), zoomed_image.data.end(),
                  [pixel_count](double& pixel) { pixel /= pixel_count; });
    return zoomed_image;
}

fftw::ComplexUPtr PeriodizationZoomStrategy::PeriodizeFFT(
      int zoom, const Image& image, fftw::ComplexUPtr image_fft) const {
    if (zoom <= 1) {
        // nothing to periodize: 1:1 zoom
        return image_fft;
    }

    int image_row_count = image.size.row;
    int image_col_count = image.size.col;

    int fft_row_count = image_row_count;
    int fft_col_count = (image_col_count / 2) + 1;

    int zoomed_row_count = image_row_count * zoom;
    int zoomed_col_count = image_col_count * zoom;
    int fft_zoomed_row_count = zoomed_row_count;
    int fft_zoomed_col_count = zoomed_col_count / 2 + 1;

    Size zoomed_fft_size(fft_zoomed_row_count, fft_zoomed_col_count);
    auto zoomed_fft = fftw::CreateComplex(zoomed_fft_size);

    auto image_fft_span = utils::MakeSmartPtrArraySpan(image_fft, image.size);
    auto zoomed_fft_span =
          utils::MakeSmartPtrArraySpan(zoomed_fft, zoomed_fft_size);
    for (int row = 0; row < fft_row_count; ++row) {
        int bottom_row = (fft_zoomed_row_count - fft_row_count + row) *
                         fft_zoomed_col_count;
        for (int col = 0; col < fft_col_count; ++col) {
            int fft_idx = row * fft_col_count + col;
            int top_left_idx = row * fft_zoomed_col_count + col;
            int bottom_left_idx = bottom_row + col;
            int top_right_idx =
                  row * fft_zoomed_col_count + 2 * fft_col_count - col - 2;
            int bottom_right_idx = bottom_row + 2 * fft_col_count - col - 2;

            int top_bottom_left_idx =
                  top_left_idx +
                  2 * (fft_row_count - row - 1) * fft_zoomed_col_count;
            int top_bottom_right_idx =
                  top_right_idx +
                  (2 * (fft_row_count - row) - 1) * fft_zoomed_col_count + 1;

            int bottom_top_left_idx =
                  (fft_zoomed_row_count - fft_row_count - 1 - row) *
                        fft_zoomed_col_count +
                  col;
            int bottom_top_right_idx =
                  bottom_top_left_idx + 2 * (fft_col_count - col) - 1;

            double real_val = image_fft_span[fft_idx][0];
            double im_val = image_fft_span[fft_idx][1];

            // copy top left corner
            zoomed_fft_span[top_left_idx][0] = real_val;
            zoomed_fft_span[top_left_idx][1] = im_val;

            // copy bottom left corner
            zoomed_fft_span[bottom_left_idx][0] = real_val;
            zoomed_fft_span[bottom_left_idx][1] = im_val;

            if (zoom != 2) {
                // copy to the top bottom left corner
                if (row == fft_row_count - 1) {
                    zoomed_fft_span[top_bottom_left_idx][0] = real_val;
                    zoomed_fft_span[top_bottom_left_idx][1] = im_val;
                } else {
                    double tmp_real_val =
                          image_fft_span[(row + 1) * fft_col_count + col][0];
                    double tmp_im_val =
                          image_fft_span[(row + 1) * fft_col_count + col][1];
                    zoomed_fft_span[top_bottom_left_idx][0] = tmp_real_val;
                    zoomed_fft_span[top_bottom_left_idx][1] = tmp_im_val;
                }

                // copy to the top bottom right corner
                if (top_bottom_right_idx <
                    fft_zoomed_col_count * (2 * fft_row_count - 1)) {
                    zoomed_fft_span[top_bottom_right_idx][0] = real_val;
                    zoomed_fft_span[top_bottom_right_idx][1] = im_val;
                }

                // copy to the bottom top right corner
                zoomed_fft_span[bottom_top_right_idx][0] = real_val;
                zoomed_fft_span[bottom_top_right_idx][1] = im_val;

                // copy to the bottom top left corner
                zoomed_fft_span[bottom_top_left_idx][0] = real_val;
                zoomed_fft_span[bottom_top_left_idx][1] = im_val;
            }

            if (col == fft_col_count - 1) {
                // duplicate extreme right pixel of each source spectrum row
                zoomed_fft_span[top_right_idx][0] = real_val;
                zoomed_fft_span[top_right_idx][1] = im_val;

                zoomed_fft_span[bottom_right_idx][0] = real_val;
                zoomed_fft_span[bottom_right_idx][1] = im_val;
            } else {
                double right_real_val = image_fft_span[fft_idx + 1][0];
                double right_im_val = image_fft_span[fft_idx + 1][1];
                // copy top right corner
                zoomed_fft_span[top_right_idx][0] = right_real_val;
                zoomed_fft_span[top_right_idx][1] = right_im_val;

                // copy bottom right corner
                zoomed_fft_span[bottom_right_idx][0] = right_real_val;
                zoomed_fft_span[bottom_right_idx][1] = right_im_val;
            }
        }
    }

    return zoomed_fft;
}

}  // namespace resampler
}  // namespace sirius
