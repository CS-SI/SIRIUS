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

#include "sirius/rotation/processor.h"
#include "sirius/rotation/utils.h"

#include <cmath>
#include <cstring>

#include "sirius/exception.h"

#include "sirius/utils/gsl.h"
#include "sirius/utils/log.h"
#include "sirius/utils/numeric.h"

#include "sirius/fftw/wrapper.h"

namespace sirius {
namespace rotation {

/*
   Implementation of the rotation in frequency domain thanks to the method
   exposed in "Fast Fourier method for the accurate rotation of sampled images"
*/

Image Processor::Process(const Image& image,
                         const Parameters& parameters) const {
    Image im;
    int angle = parameters.angle;
    if (image.size.row <= image.size.col) {
        im = Image({2 * image.size.row, 2 * image.size.col});
    } else {
        im = Image({3 * image.size.row, 3 * image.size.col});
    }

    Point center(std::floor(im.size.col / 2.0), std::floor(im.size.row / 2.0));

    // center source image in the padded one
    int begin_row = center.y - image.size.row / 2;
    int end_row = begin_row + image.size.row;
    int begin_col = center.x - image.size.col / 2;
    auto src_data = image.data.data();
    auto dst_data = im.data.data() + begin_row * im.size.col + begin_col;

    for (int i = begin_row; i < end_row; ++i) {
        std::copy(src_data, src_data + image.size.col, dst_data);
        src_data += image.size.col;
        dst_data += im.size.col;
    }

    // get minimal size that contains the rotated image
    auto min_size = ComputeNonRotatedHull(image.size, angle);

    // get the shift vector to recover top left corner coordinates from
    // the center
    auto hypotenuse =
          std::sqrt(std::pow(min_size.row, 2) + std::pow(min_size.col, 2));
    auto angle_diag_rad = std::acos(min_size.col / hypotenuse);
    Size shift;
    if (angle == 90 || angle == -90) {
        shift = {static_cast<int>(std::ceil(hypotenuse / 2.0) *
                                  std::sin(angle_diag_rad)),
                 static_cast<int>(std::ceil(hypotenuse / 2.0) *
                                        std::cos(angle_diag_rad) -
                                  1)};
    } else {
        shift = {static_cast<int>(std::ceil(hypotenuse / 2.0) *
                                  std::sin(angle_diag_rad)),
                 static_cast<int>(std::ceil(hypotenuse / 2.0) *
                                  std::cos(angle_diag_rad))};
    }

    auto teta = (angle * M_PI) / 180.0;
    auto a = std::tan(teta / 2.0);
    auto b = -std::sin(teta);

    int begin_x_range = -std::floor(im.size.row / 2.0);
    int end_x_range = std::ceil(im.size.row / 2.0);
    int begin_y_range = -std::floor(im.size.col / 2.0);
    int end_y_range = std::ceil(im.size.col / 2.0);

    std::vector<int> x_range;
    std::vector<int> y_range;
    for (int i = begin_x_range; i < end_x_range; ++i) {
        x_range.push_back(i);
    }
    for (int i = begin_y_range; i < end_y_range; ++i) {
        y_range.push_back(i);
    }

    std::vector<int> nx(x_range.size());
    std::vector<int> ny(y_range.size());
    utils::IFFTShift(x_range.data(), x_range.size(), nx.data());
    utils::IFFTShift(y_range.data(), y_range.size(), ny.data());

    auto M = im.size.row;
    auto N = im.size.col;

    Image ix_real({static_cast<int>(nx.size()), static_cast<int>(ny.size())});
    // x translation
    for (int i = 0; i < M; ++i) {
        // fft one row of padded image
        std::vector<double> vec_im(N);
        std::copy(im.data.begin() + i * N, im.data.begin() + (i + 1) * N,
                  vec_im.data());
        auto fft_vec_im = fftw::FFT(vec_im.data(), {1, N});

        auto ix_cplx =
              fftw::CreateComplex({1, static_cast<int>(ny.size() / 2 + 1)});
        // (a + ib) * exp(i * pi * teta)
        for (int j = 0; j < N / 2 + 1; ++j) {
            double val_real = fft_vec_im.get()[j][0] *
                                    cos(-2 * M_PI * (i - std::floor(M / 2.0)) *
                                        ny[j] * a / N) -
                              fft_vec_im.get()[j][1] *
                                    sin(-2 * M_PI * (i - std::floor(M / 2.0)) *
                                        ny[j] * a / N);
            double val_im = fft_vec_im.get()[j][0] *
                                  sin(-2 * M_PI * (i - std::floor(M / 2.0)) *
                                      ny[j] * a / N) +
                            fft_vec_im.get()[j][1] *
                                  cos(-2 * M_PI * (i - std::floor(M / 2.0)) *
                                      ny[j] * a / N);
            ix_cplx.get()[j][0] = val_real;
            ix_cplx.get()[j][1] = val_im;
        }
        auto ix =
              fftw::IFFT({1, static_cast<int>(ny.size())}, std::move(ix_cplx));
        // normalize ifft result
        int pixel_count = ix.CellCount();
        std::for_each(ix.data.begin(), ix.data.end(),
                      [pixel_count](double& pixel) { pixel /= pixel_count; });
        std::copy(ix.data.begin(), ix.data.end(),
                  ix_real.data.begin() + (i * N));
    }

    Image iy_real({static_cast<int>(nx.size()), static_cast<int>(ny.size())});
    // y translation
    for (int i = 0; i < N; ++i) {
        // fft one col of ix image
        std::vector<double> vec_im(M);
        for (int j = 0; j < M; ++j) {
            vec_im[j] = ix_real.data[j * N + i];
        }

        auto fft_vec_im = fftw::FFT(vec_im.data(), {M, 1});
        auto iy_cplx = fftw::CreateComplex({static_cast<int>(nx.size()), 1});
        for (int j = 0; j < M; ++j) {
            double val_real = fft_vec_im.get()[j][0] *
                                    cos(-2 * M_PI * (i - std::floor(N / 2.0)) *
                                        nx[j] * b / M) -
                              fft_vec_im.get()[j][1] *
                                    sin(-2 * M_PI * (i - std::floor(N / 2.0)) *
                                        nx[j] * b / M);
            double val_im = fft_vec_im.get()[j][0] *
                                  sin(-2 * M_PI * (i - std::floor(N / 2.0)) *
                                      nx[j] * b / M) +
                            fft_vec_im.get()[j][1] *
                                  cos(-2 * M_PI * (i - std::floor(N / 2.0)) *
                                      nx[j] * b / M);
            iy_cplx.get()[j][0] = val_real;
            iy_cplx.get()[j][1] = val_im;
        }

        auto iy =
              fftw::IFFT({static_cast<int>(nx.size()), 1}, std::move(iy_cplx));
        int pixel_count = iy.CellCount();
        std::for_each(iy.data.begin(), iy.data.end(),
                      [pixel_count](double& pixel) { pixel /= pixel_count; });
        for (int j = 0; j < M; ++j) {
            iy_real.data[j * im.size.col + i] = iy.data[j];
        }
    }

    Image rotated_im(
          {static_cast<int>(nx.size()), static_cast<int>(ny.size())});
    // x translation again
    for (int i = 0; i < M; ++i) {
        // fft one row of iy image
        std::vector<double> vec_im(N);
        std::copy(iy_real.data.begin() + i * N,
                  iy_real.data.begin() + (i + 1) * N, vec_im.data());
        auto fft_vec_im = fftw::FFT(vec_im.data(), {1, N});

        auto rotated_im_cplx =
              fftw::CreateComplex({1, static_cast<int>(ny.size() / 2 + 1)});
        for (int j = 0; j < N / 2 + 1; ++j) {
            double val_real = fft_vec_im.get()[j][0] *
                                    cos(-2 * M_PI * (i - std::floor(M / 2.0)) *
                                        ny[j] * a / N) -
                              fft_vec_im.get()[j][1] *
                                    sin(-2 * M_PI * (i - std::floor(M / 2.0)) *
                                        ny[j] * a / N);
            double val_im = fft_vec_im.get()[j][0] *
                                  sin(-2 * M_PI * (i - std::floor(M / 2.0)) *
                                      ny[j] * a / N) +
                            fft_vec_im.get()[j][1] *
                                  cos(-2 * M_PI * (i - std::floor(M / 2.0)) *
                                      ny[j] * a / N);
            rotated_im_cplx.get()[j][0] = val_real;
            rotated_im_cplx.get()[j][1] = val_im;
        }
        auto rotated_im_row = fftw::IFFT({1, static_cast<int>(ny.size())},
                                         std::move(rotated_im_cplx));
        int pixel_count = rotated_im_row.CellCount();
        std::for_each(rotated_im_row.data.begin(), rotated_im_row.data.end(),
                      [pixel_count](double& pixel) { pixel /= pixel_count; });
        std::copy(rotated_im_row.data.begin(), rotated_im_row.data.end(),
                  rotated_im.data.begin() + (i * N));
    }

    Image output_image(min_size);
    Point top_left(static_cast<int>(std::ceil(center.x - shift.col)),
                   static_cast<int>(std::ceil(center.y - shift.row)));

    // crop rotated image to its rectangular hull
    int offset = top_left.y * rotated_im.size.col + top_left.x;
    int offset_out = 0;
    for (int i = center.y; i < center.y + min_size.row; ++i) {
        std::copy(rotated_im.data.begin() + offset,
                  rotated_im.data.begin() + offset + min_size.col,
                  output_image.data.data() + offset_out);
        offset_out += min_size.col;
        offset += rotated_im.size.col;
    }

    return output_image;
}
}  // namespace rotation
}  // namespace sirius