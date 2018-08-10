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

#include "sirius/translation/processor.h"

#include <cstring>

#include "sirius/exception.h"

#include "sirius/translation/utils.h"

#include "sirius/utils/gsl.h"
#include "sirius/utils/log.h"
#include "sirius/utils/numeric.h"

#include "sirius/fftw/wrapper.h"

namespace sirius {
namespace translation {

Image Processor::Process(const Image& image,
                         const Parameters& parameters) const {
    float row_shift = parameters.row_shift;
    float col_shift = parameters.col_shift;

    if (row_shift >= image.size.row || col_shift >= image.size.col ||
        row_shift <= -image.size.row || col_shift <= -image.size.col) {
        LOG("translation_processor", warn,
            "Desired shift x:{}, y:{} is greater than image size, this shift "
            "will be applied instead x:{}, y:{}",
            col_shift, row_shift,
            col_shift / static_cast<float>(image.size.col),
            row_shift / static_cast<float>(image.size.row));
        row_shift /= static_cast<float>(image.size.row);
        col_shift /= static_cast<float>(image.size.col);
    }

    // LOG("sirius", trace, "Translation x:{}, y:{}", col_shift_, row_shift_);

    float reduced_x_shift = col_shift - static_cast<int>(col_shift);
    float reduced_y_shift = row_shift - static_cast<int>(row_shift);

    Image output_image(image.size);

    // sub_pixel translation required
    if (reduced_x_shift != 0.0 || reduced_y_shift != 0.0) {
        Image shifted_image(image.size);

        utils::IFFTShift2D(image.data.data(), image.size,
                           shifted_image.data.data());

        auto fft_image =
              fftw::FFT(shifted_image.data.data(), shifted_image.size);
        int fft_row_count = shifted_image.size.row;
        int fft_col_count = shifted_image.size.col / 2 + 1;

        // get frequencies for which fft was calculated
        std::vector<double> freq_y =
              utils::ComputeFFTFreq(shifted_image.size.col);
        std::vector<double> freq_x =
              utils::ComputeFFTFreq(shifted_image.size.row, false);

        auto exp_cplx_y = fftw::CreateComplex({1, fft_col_count});
        auto exp_cplx_x = fftw::CreateComplex({fft_row_count, 1});

        // calculate 1D shift vectors
        for (int j = 0; j < fft_col_count; ++j) {
            exp_cplx_y.get()[j][0] =
                  cos(-2 * M_PI * reduced_x_shift * freq_y[j]);
            exp_cplx_y.get()[j][1] =
                  sin(-2 * M_PI * reduced_x_shift * freq_y[j]);
        }

        for (int i = 0; i < fft_row_count; ++i) {
            exp_cplx_x.get()[i][0] =
                  cos(-2 * M_PI * reduced_y_shift * freq_x[i]);
            exp_cplx_x.get()[i][1] =
                  sin(-2 * M_PI * reduced_y_shift * freq_x[i]);
        }

        auto fft_image_span =
              utils::MakeSmartPtrArraySpan(fft_image, image.size);
        // apply 2D shift on real and imaginary parts
        for (int i = 0; i < fft_row_count; ++i) {
            for (int j = 0; j < fft_col_count; ++j) {
                int idx = i * fft_col_count + j;
                auto tmp_real = fft_image_span[idx][0];
                auto tmp_im = fft_image_span[idx][1];
                fft_image_span[idx][0] =
                      fft_image_span[idx][0] * exp_cplx_x[i][0] *
                            exp_cplx_y[j][0] -
                      fft_image_span[idx][1] * exp_cplx_x[i][1] *
                            exp_cplx_y[j][0] -
                      fft_image_span[idx][1] * exp_cplx_x[i][0] *
                            exp_cplx_y[j][1] -
                      fft_image_span[idx][0] * exp_cplx_x[i][1] *
                            exp_cplx_y[j][1];

                fft_image_span[idx][1] =
                      tmp_real * exp_cplx_x[i][0] * exp_cplx_y[j][1] -
                      tmp_im * exp_cplx_x[i][1] * exp_cplx_y[j][1] +
                      tmp_im * exp_cplx_x[i][0] * exp_cplx_y[j][0] +
                      tmp_real * exp_cplx_x[i][1] * exp_cplx_y[j][0];
            }
        }

        shifted_image = fftw::IFFT(shifted_image.size, std::move(fft_image));

        utils::FFTShift2D(shifted_image.data.data(), output_image.size,
                          output_image.data.data());

        // normalize output image
        int pixel_count = output_image.CellCount();
        std::for_each(output_image.data.begin(), output_image.data.end(),
                      [pixel_count](double& pixel) { pixel /= pixel_count; });
    } else {
        output_image = image;
    }

    // truncate borders on the opposite side of where it should have been
    // duplicated if we had applied the complete shift
    output_image = RemoveBorders(output_image, static_cast<int>(-row_shift),
                                 static_cast<int>(-col_shift));

    if (reduced_x_shift < 0) {
        reduced_x_shift = std::floor(reduced_x_shift);
    } else {
        reduced_x_shift = std::ceil(reduced_x_shift);
    }

    if (reduced_y_shift < 0) {
        reduced_y_shift = std::floor(reduced_y_shift);
    } else {
        reduced_y_shift = std::ceil(reduced_y_shift);
    }
    // remove, at max, 1 row and col (sub pixel shift)
    return RemoveBorders(output_image, reduced_y_shift, reduced_x_shift);
}

}  // namespace translation
}  // namespace sirius