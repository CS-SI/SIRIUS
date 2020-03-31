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

#ifndef SIRIUS_IMAGE_DECOMPOSITION_PERIODIC_SMOOTH_POLICY_TXX_
#define SIRIUS_IMAGE_DECOMPOSITION_PERIODIC_SMOOTH_POLICY_TXX_

#include "sirius/image_decomposition/periodic_smooth_policy.h"

#include "sirius/fftw/fftw.h"
#include "sirius/fftw/types.h"
#include "sirius/fftw/wrapper.h"

#include "sirius/utils/gsl.h"

namespace sirius {
namespace image_decomposition {

template <typename Transformation, typename ImageProcessor,
          typename ImageInterpolator>
Image PeriodicSmoothPolicy<Transformation, ImageProcessor, ImageInterpolator>::
      DecomposeAndProcess(
            const Image& image,
            const typename Transformation::Parameters& parameters) const {
    // 1) compute intensity changes between two opposite borders
    LOG("periodic_smooth_decomposition", trace, "compute intensity changes");
    Image border_intensity_changes(image.size);
    auto border_intensity_changes_span =
          gsl::as_multi_span(border_intensity_changes.data);
    // last line - first line, first line - last line
    for (int j = 0; j < image.size.col; j++) {
        // (last line - first line) placed in the first line of
        // border_intensity_changes
        border_intensity_changes_span[j] +=
              image.data[(image.size.row - 1) * image.size.col + j] -
              image.data[j];
        // (first line - last line) placed the last line of
        // border_intensity_changes
        border_intensity_changes_span[(image.size.row - 1) * image.size.col +
                                      j] +=
              image.data[j] -
              image.data[(image.size.row - 1) * image.size.col + j];
    }

    for (int i = 0; i < image.size.row; i++) {
        // (last col - first col) placed in the first col of
        // border_intensity_changes
        border_intensity_changes_span[i * image.size.col] +=
              image.data[(i + 1) * image.size.col - 1] -
              image.data[i * image.size.col];
        // (first col - last col) placed in the last col of
        // border_intensity_changes
        border_intensity_changes_span[(i + 1) * image.size.col - 1] +=
              image.data[i * image.size.col] -
              image.data[(i + 1) * image.size.col - 1];
    }

    Size fft_size(image.size.row, image.size.col / 2 + 1);

    // 2) fft of intensity changes
    LOG("periodic_smooth_decomposition", trace,
        "compute intensity changes FFT");
    auto intensity_fft = fftw::FFT(border_intensity_changes);
    auto intensity_fft_span =
          utils::MakeSmartPtrArraySpan(intensity_fft, fft_size);

    // 3) compute smooth part of the image
    LOG("periodic_smooth_decomposition", trace, "compute smooth part");
    std::vector<double> cosx(fft_size.CellCount(), 0);
    std::vector<double> cosy(fft_size.CellCount(), 0);
    for (int i = 0; i < fft_size.row; i++) {
        for (int j = 0; j < fft_size.col; j++) {
            cosx[i * fft_size.col + j] =
                  2.0 * cos(2 * M_PI * j / static_cast<double>(image.size.col));
            cosy[i * fft_size.col + j] =
                  2.0 * cos(2 * M_PI * i / static_cast<double>(image.size.row));
        }
    }

    auto smooth_part_fft = fftw::CreateComplex(fft_size);
    auto smooth_part_fft_span =
          utils::MakeSmartPtrArraySpan(smooth_part_fft, fft_size);
    smooth_part_fft_span[0][0] = 0;
    smooth_part_fft_span[0][1] = 0;
    for (int i = 1; i < fft_size.row; i++) {
        for (int j = 0; j < fft_size.col; j++) {
            int fft_index = i * fft_size.col + j;
            smooth_part_fft_span[fft_index][0] =
                  intensity_fft_span[fft_index][0] /
                  (cosx[fft_index] + cosy[fft_index] - 4.0);
            smooth_part_fft_span[fft_index][1] =
                  intensity_fft_span[fft_index][1] /
                  (cosx[fft_index] + cosy[fft_index] - 4.0);
        }
    }

    for (int j = 1; j < fft_size.col; j++) {
        smooth_part_fft_span[j][0] =
              intensity_fft_span[j][0] / (cosx[j] + cosy[j] - 4.0);
        smooth_part_fft_span[j][1] =
              intensity_fft_span[j][1] / (cosx[j] + cosy[j] - 4.0);
    }

    // 4) fft input image
    LOG("periodic_smooth_decomposition", trace, "compute image FFT");
    auto image_fft = fftw::FFT(image);
    auto image_fft_span = utils::MakeSmartPtrArraySpan(image_fft, fft_size);

    // 5) compute periodic part of the image
    LOG("periodic_smooth_decomposition", trace, "compute periodic part");
    auto periodic_part_fft = fftw::CreateComplex(fft_size);
    auto periodic_part_fft_span =
          utils::MakeSmartPtrArraySpan(periodic_part_fft, fft_size);
    auto fft_count = fft_size.CellCount();
    for (int i = 0; i < fft_count; ++i) {
        periodic_part_fft_span[i][0] =
              image_fft_span[i][0] - smooth_part_fft_span[i][0];
        periodic_part_fft_span[i][1] =
              image_fft_span[i][1] - smooth_part_fft_span[i][1];
    }

    // 6) ifft periodic part
    LOG("periodic_smooth_decomposition", trace, "compute periodic part IFFT");
    auto periodic_part_image =
          fftw::IFFT(image.size, std::move(periodic_part_fft));

    // 7) apply processor on periodic part
    LOG("periodic_smooth_decomposition", trace, "process periodic part");
    // method inherited from ImageProcessor
    auto processed_image = this->Process(periodic_part_image, parameters);

    // 8) ifft smooth part
    LOG("periodic_smooth_decomposition", trace, "smooth part IFFT");
    auto smooth_part_image = fftw::IFFT(image.size, std::move(smooth_part_fft));

    // 9) normalize smooth_part_image
    LOG("periodic_smooth_decomposition", trace, "normalize smooth image part");
    int image_cell_count = image.CellCount();
    std::for_each(
          smooth_part_image.data.begin(), smooth_part_image.data.end(),
          [image_cell_count](double& cell) { cell /= image_cell_count; });

    // 10) interpolate 2d smooth part image
    LOG("periodic_smooth_decomposition", trace,
        "interpolate smooth image part");
    // method inherited from ImageInterpolator
    auto interpolated_smooth_image =
          this->Interpolate2D(smooth_part_image, parameters);

    // 11) normalize periodic_part_image
    LOG("periodic_smooth_decomposition", trace,
        "normalize periodic image part");
    std::for_each(
          processed_image.data.begin(), processed_image.data.end(),
          [image_cell_count](double& cell) { cell /= image_cell_count; });

    // 12) sum periodic and smooth parts
    LOG("periodic_smooth_decomposition", trace,
        "sum periodic and smooth image parts");
    Image output_image(processed_image.size);
    for (int i = 0; i < processed_image.size.row; i++) {
        for (int j = 0; j < processed_image.size.col; j++) {
            output_image.Set(i, j,
                             processed_image.Get(i, j) +
                                   interpolated_smooth_image.Get(i, j));
        }
    }

    return output_image;
}

}  // namespace image_decomposition
}  // namespace sirius

#endif  // SIRIUS_IMAGE_DECOMPOSITION_PERIODIC_SMOOTH_POLICY_TXX_
