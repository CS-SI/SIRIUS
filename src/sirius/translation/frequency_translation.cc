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

#include "sirius/translation/frequency_translation.h"

#include <cstring>

#include "sirius/exception.h"

#include "sirius/utils/gsl.h"
#include "sirius/utils/log.h"
#include "sirius/utils/numeric.h"

#include "sirius/fftw/wrapper.h"

namespace sirius {

Image FrequencyTranslation::Shift(const Image& image, float shift_col,
                                  float shift_row) {
    LOG("sirius", info, "apply translation to the image");
    Image shifted_image(image.size);

    utils::IFFTShift2D(image.data.data(), image.size,
                       shifted_image.data.data());

    auto fft_image = fftw::FFT(shifted_image.data.data(), shifted_image.size);
    int fft_row_count = shifted_image.size.row;
    int fft_col_count = shifted_image.size.col / 2 + 1;

    // get frequencies for which fft was calculated
    std::vector<double> freq_y = utils::ComputeFFTFreq(shifted_image.size.col);
    std::vector<double> freq_x =
          utils::ComputeFFTFreq(shifted_image.size.row, false);

    auto exp_cplx_y = fftw::CreateComplex({1, fft_col_count});
    auto exp_cplx_x = fftw::CreateComplex({fft_row_count, 1});

    // calculate 1D shift vectors
    for (int j = 0; j < fft_col_count; ++j) {
        exp_cplx_y.get()[j][0] = cos(-2 * M_PI * shift_col * freq_y[j]);
        exp_cplx_y.get()[j][1] = sin(-2 * M_PI * shift_col * freq_y[j]);
    }

    for (int i = 0; i < fft_row_count; ++i) {
        exp_cplx_x.get()[i][0] = cos(-2 * M_PI * shift_row * freq_x[i]);
        exp_cplx_x.get()[i][1] = sin(-2 * M_PI * shift_row * freq_x[i]);
    }

    auto fft_image_span = utils::MakeSmartPtrArraySpan(fft_image, image.size);
    // apply 2D shift on real and imaginary parts
    for (int i = 0; i < fft_row_count; ++i) {
        for (int j = 0; j < fft_col_count; ++j) {
            int idx = i * fft_col_count + j;
            auto tmp_real = fft_image_span[idx][0];
            auto tmp_im = fft_image_span[idx][1];
            fft_image_span[idx][0] =
                  fft_image_span[idx][0] * exp_cplx_x[i][0] * exp_cplx_y[j][0] -
                  fft_image_span[idx][1] * exp_cplx_x[i][1] * exp_cplx_y[j][0] -
                  fft_image_span[idx][1] * exp_cplx_x[i][0] * exp_cplx_y[j][1] -
                  fft_image_span[idx][0] * exp_cplx_x[i][1] * exp_cplx_y[j][1];

            fft_image_span[idx][1] =
                  tmp_real * exp_cplx_x[i][0] * exp_cplx_y[j][1] -
                  tmp_im * exp_cplx_x[i][1] * exp_cplx_y[j][1] +
                  tmp_im * exp_cplx_x[i][0] * exp_cplx_y[j][0] +
                  tmp_real * exp_cplx_x[i][1] * exp_cplx_y[j][0];
        }
    }

    shifted_image = fftw::IFFT(shifted_image.size, std::move(fft_image));
    Image unshifted_image(shifted_image.size);

    utils::FFTShift2D(shifted_image.data.data(), unshifted_image.size,
                      unshifted_image.data.data());

    // normalize output image
    int pixel_count = unshifted_image.CellCount();
    std::for_each(unshifted_image.data.begin(), unshifted_image.data.end(),
                  [pixel_count](double& pixel) { pixel /= pixel_count; });

    unshifted_image = RemoveBorders(unshifted_image, std::ceil(shift_col),
                                    std::ceil(shift_row));

    return unshifted_image;
}

Image FrequencyTranslation::RemoveBorders(const Image& image, int shift_col,
                                          int shift_row) {
    LOG("FrequencyTranslation", trace, "Remove borders");
    Image output_image(
          {image.size.row - shift_row, image.size.col - shift_col});

    int begin_row, begin_col, end_row, end_col;
    if (shift_row > 0) {
        begin_row = shift_row;
        end_row = image.size.row;
    } else {
        begin_row = 0;
        end_row = image.size.row - shift_row;
    }

    if (shift_col > 0) {
        begin_col = shift_col;
        end_col = image.size.col;
    } else {
        begin_col = 0;
        end_col = image.size.col - shift_col;
    }

    int begin_src = begin_row * image.size.col + begin_col;
    int nb_elem = end_col - begin_col;
    int begin = 0;
    for (int i = begin_row; i < end_row; ++i) {
        memcpy(&output_image.data[begin], &image.data[begin_src],
               nb_elem * sizeof(double));
        begin_src += image.size.col;
        begin += nb_elem;
    }

    return output_image;
}

}  // end namespace sirius