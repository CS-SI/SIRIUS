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

#include "sirius/utils/numeric.h"

#include <cmath>

#include <algorithm>
#include <utility>

#include "sirius/utils/log.h"

namespace sirius {
namespace utils {

int Gcd(int a, int b) {
    a = std::abs(a);
    b = std::abs(b);
    while (a != b) {
        if (a > b)
            a -= b;
        else
            b -= a;
    }
    return a;
}

void FFTShift2D(const double* data, const Size& size, double* shifted_data) {
    int row_shift = size.row / 2;
    int col_shift = size.col / 2;

    for (int row = 0; row < size.row; ++row) {
        int shifted_row = (row + row_shift) % size.row;
        for (int col = 0; col < size.col; ++col) {
            int shifted_col = (col + col_shift) % size.col;
            shifted_data[shifted_col + size.col * shifted_row] =
                  data[col + size.col * row];
        }
    }
}

void IFFTShift2D(const double* data, const Size& size, double* shifted_data) {
    int row_shift = std::ceil(static_cast<double>(size.row) / 2);
    int col_shift = std::ceil(static_cast<double>(size.col) / 2);

    for (int row = 0; row < size.row; ++row) {
        int shifted_row = (row + row_shift) % size.row;
        for (int col = 0; col < size.col; ++col) {
            int shifted_col = (col + col_shift) % size.col;
            shifted_data[shifted_row * size.col + shifted_col] =
                  data[row * size.col + col];
        }
    }
}

void IFFTShift2DUncentered(const double* data, const Size& size,
                           const Point& hot_point, double* shifted_data) {
    Size block_4(size.row - hot_point.y, size.col - hot_point.x);
    Size block_3(block_4.row, hot_point.x);
    Size block_2(hot_point.y, block_4.col);
    Size block_1(hot_point.y, hot_point.x);

    Point p4_shifted(0, 0);  // block4 top left corner after shift
    Point p3_shifted(block_4.col, 0);
    Point p2_shifted(0, block_4.row);
    Point p1_shifted(block_4.col, block_4.row);

    Point p4 = hot_point;  // block4 top left corner in source image
    Point p3(0, size.row - block_4.row);
    Point p2(p4.x, 0);
    Point p1(0, 0);

    int begin = p4_shifted.x + p4_shifted.y * size.col;
    int begin_src = p4.x + p4.y * size.col;
    for (int i = 0; i < block_4.row; ++i) {
        std::copy(data + begin_src, data + (begin_src + block_4.col),
                  shifted_data + begin);
        begin_src += size.col;
        begin += size.col;
    }

    begin = p3_shifted.x + p3_shifted.y * size.col;
    begin_src = p3.x + p3.y * size.col;
    for (int i = 0; i < block_3.row; ++i) {
        std::copy(data + begin_src, data + (begin_src + block_3.col),
                  shifted_data + begin);
        begin_src += size.col;
        begin += size.col;
    }

    begin = p2_shifted.x + p2_shifted.y * size.col;
    begin_src = p2.x + p2.y * size.col;
    for (int i = 0; i < block_2.row; ++i) {
        std::copy(data + begin_src, data + (begin_src + block_2.col),
                  shifted_data + begin);
        begin_src += size.col;
        begin += size.col;
    }

    begin = p1_shifted.x + p1_shifted.y * size.col;
    begin_src = p1.x + p1.y * size.col;
    for (int i = 0; i < block_1.row; ++i) {
        std::copy(data + begin_src, data + (begin_src + block_1.col),
                  shifted_data + begin);
        begin_src += size.col;
        begin += size.col;
    }
}

void FFTShift2DUncentered(const double* data, const Size& size,
                          const Point& hot_point, double* shifted_data) {
    Size block_4(size.row - hot_point.y, size.col - hot_point.x);
    Size block_3(block_4.row, hot_point.x);
    Size block_2(hot_point.y, block_4.col);
    Size block_1(hot_point.y, hot_point.x);

    Point p4(0, 0);  // block4 top left corner in source image
    Point p3(block_4.col, 0);
    Point p2(0, block_4.row);
    Point p1(block_4.col, block_4.row);

    Point p4_shifted = hot_point;  // block4 top left corner after shift
    Point p3_shifted(0, size.row - block_4.row);
    Point p2_shifted(p4_shifted.x, 0);
    Point p1_shifted(0, 0);

    int begin = p4_shifted.x + p4_shifted.y * size.col;
    int begin_src = p4.x + p4.y * size.col;
    for (int i = 0; i < block_4.row; ++i) {
        std::copy(data + begin_src, data + (begin_src + block_4.col),
                  shifted_data + begin);
        begin += size.col;
        begin_src += size.col;
    }

    begin = p3_shifted.x + p3_shifted.y * size.col;
    begin_src = p3.x + p3.y * size.col;
    for (int i = 0; i < block_3.row; ++i) {
        std::copy(data + begin_src, data + (begin_src + block_3.col),
                  shifted_data + begin);
        begin += size.col;
        begin_src += size.col;
    }

    begin = p2_shifted.x + p2_shifted.y * size.col;
    begin_src = p2.x + p2.y * size.col;
    for (int i = 0; i < block_2.row; ++i) {
        std::copy(data + begin_src, data + (begin_src + block_2.col),
                  shifted_data + begin);
        begin += size.col;
        begin_src += size.col;
    }

    begin = p1_shifted.x + p1_shifted.y * size.col;
    begin_src = p1.x + p1.y * size.col;
    for (int i = 0; i < block_1.row; ++i) {
        std::copy(data + begin_src, data + (begin_src + block_1.col),
                  shifted_data + begin);
        begin += size.col;
        begin_src += size.col;
    }
}

Size GenerateDyadicSize(const Size& size, const int res_in,
                        const Size& padding_size) {
    int h = size.row;
    int w = size.col;
    int initial_h = size.row;
    int initial_w = size.col;

    while (std::floor(std::log2((h + padding_size.row) * res_in)) !=
           std::ceil(std::log2((h + padding_size.row) * res_in))) {
        h++;
        if (h > 100 * initial_h) {
            LOG("numeric", warn,
                "Could not resize block. Initial size will be used");
            return {initial_h, initial_w};
        }
    }

    while (std::floor(std::log2(w + padding_size.col) * res_in) !=
           std::ceil(std::log2(w + padding_size.col) * res_in)) {
        w++;
        if (w > 100 * initial_w) {
            LOG("numeric", warn,
                "Could not resize block. Initial size will be used");
            return {initial_h, initial_w};
        }
    }

    return {h, w};
}

Size GenerateZoomCompliantSize(const Size& size, const ZoomRatio& zoom_r) {
    int h = size.row;
    int w = size.col;
    int initial_h = size.row;
    int initial_w = size.col;

    while (std::floor(h * zoom_r.input_resolution() /
                      static_cast<double>(zoom_r.output_resolution())) !=
           std::ceil(h * zoom_r.input_resolution() /
                     static_cast<double>(zoom_r.output_resolution()))) {
        h++;
        if (h > 100 * initial_h) {
            LOG("numeric", warn,
                "Could not resize block to be compliant with given ratio {}. "
                "Output image may contain disjoint blocks",
                zoom_r.ratio());
            return {initial_h, initial_w};
        }
    }

    while (std::floor(w * zoom_r.input_resolution() /
                      static_cast<double>(zoom_r.output_resolution())) !=
           std::ceil(w * zoom_r.input_resolution() /
                     static_cast<double>(zoom_r.output_resolution()))) {
        w++;
        if (w > 100 * initial_w) {
            LOG("numeric", warn,
                "Could not resize block to be compliant with given ratio {}. "
                "Output image may contain disjoint blocks",
                zoom_r.ratio());
            return {initial_h, initial_w};
        }
    }

    return {h, w};
}

std::vector<double> ComputeFFTFreq(const int n_samples, const bool half) {
    std::vector<double> freq;

    if (n_samples % 2 != 0) {
        for (int i = 0; i < (n_samples + 1) / 2; ++i) {
            freq.push_back(i / static_cast<double>(n_samples));
        }
        if (!half) {
            for (int i = 0; i < n_samples / 2; ++i) {
                freq.push_back(-((n_samples / 2) - i) /
                               static_cast<double>(n_samples));
            }
        }
    } else {
        for (int i = 0; i < n_samples / 2; ++i) {
            freq.push_back(i / static_cast<double>(n_samples));
        }
        if (!half) {
            for (int i = 0; i < n_samples / 2; ++i) {
                freq.push_back(-((n_samples / 2) - i) /
                               static_cast<double>(n_samples));
            }
        }
    }

    return freq;
}

void CreateMeshgrid(const int x_min, const int x_max, const int y_min,
                    const int y_max, std::vector<int>& xx,
                    std::vector<int>& yy) {
    for (int i = x_min; i < x_max; ++i) {
        for (int j = y_min; j < y_max; ++j) {
            xx.push_back(j);
            yy.push_back(i);
        }
    }
}

}  // namespace utils
}  // namespace sirius
