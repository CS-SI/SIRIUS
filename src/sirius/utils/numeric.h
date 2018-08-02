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

#ifndef SIRIUS_UTILS_NUMERIC_H_
#define SIRIUS_UTILS_NUMERIC_H_

#include "sirius/types.h"

namespace sirius {
namespace utils {

/**
 * \brief Compute Gcd of a and b
 *        using C++17: will be replaced with std::gcd
 */
int Gcd(int a, int b);

/**
 * \brief FFTShift 2D matrix
 */
void FFTShift2D(const double* data, const Size& size, double* shifted_data);

/**
 * \brief IFFTShift 2D matrix
 */
void IFFTShift2D(const double* data, const Size& size, double* shifted_data);

/**
 * \brief IFFTShift 2D matrix in which hot point is not centered
 * \param data input data
 * \param size dimensions of the image to be shifted
 * \param hot_point hot point coordinates
 * \param shifted_data output data
 */
void IFFTShift2DUncentered(const double* data, const Size& size,
                           const Point& hot_point, double* shifted_data);

/**
 * \brief FFTShift 2D matrix in which hot point must remain uncentered after
 * shift
 * \param data input data
 * \param size dimensions of the image to be shifted
 * \param hot_point hot point coordinates
 * \param shifted_data output data
 */
void FFTShift2DUncentered(const double* data, const Size& size,
                          const Point& hot_point, double* shifted_data);

/**
 * \brief Compute frequencies for which fft will be calculated
 * \param n_samples width of the spatial signal (expected to be odd)
 * \param half decide to return only half the frequencies
 * \return frequencies to be calculated
 */
std::vector<double> ComputeFFTFreq(int n_samples, bool half = true);

/**
 * \brief Rounds the parameters so log2(param) is an int
 * \param size size to be resized
 * \param res_in zoom coefficient that will be applied to the fft
 * \param padding_size size of the margins
 * \return new size
 */
Size GenerateDyadicSize(const Size& size, const int res_in,
                        const Size& padding_size);

/**
 * \brief Resize given dimensions so it matches with given zoom ratio
 * \param size size to be resized
 * \param zoom_r zoom ratio
 * \return zoom compliant sizes
 */
Size GenerateZoomCompliantSize(const Size& size, const ZoomRatio& zoom_r);

/**
 * \brief Create coordinates vector
 * \param x_min beginning of x axis
 * \param x_max end of x axis (x_max is excluded from the interval)
 * \param y_min beginning of y axis
 * \param y_max end of y axis (y_max is excluded from the interval)
 * \param xx output 2d x grid
 * \param yy output 2d y grid
 */
void CreateMeshgrid(int x_min, int x_max, int y_min, int y_max,
                    std::vector<int>& xx, std::vector<int>& yy);

}  // namespace utils
}  // namespace sirius

#endif  // SIRIUS_UTILS_NUMERIC_H_
