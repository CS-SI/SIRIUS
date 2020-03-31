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

#include "sirius/translation/interpolator.h"

#include <cstring>

#include "sirius/exception.h"

#include "sirius/translation/utils.h"

#include "sirius/utils/gsl.h"
#include "sirius/utils/log.h"
#include "sirius/utils/numeric.h"

#include "sirius/fftw/wrapper.h"

namespace sirius {
namespace translation {

Image Interpolator::Interpolate2D(const Image& image,
                                  const Parameters& parameters) const {
    int row_shift = parameters.row_shift;
    int col_shift = parameters.col_shift;

    LOG("translation_interpolator", trace, "input image size {}x{}",
        image.size.row, image.size.col);
    Image interpolated_im(image.size);
    interpolated_im = image;

    float beta = row_shift - static_cast<int>(row_shift);
    float alpha = col_shift - static_cast<int>(col_shift);
    LOG("translation_interpolator", trace, "alpha = {}, beta={}", alpha, beta);
    std::vector<double> BLN_kernel(4, 0);
    BLN_kernel[0] = (1 - alpha) * (1 - beta);
    BLN_kernel[1] = (1 - beta) * alpha;
    BLN_kernel[2] = beta * (1 - alpha);
    BLN_kernel[3] = alpha * beta;

    Size img_mirror_size(image.size.row + 1, image.size.col + 1);

    std::vector<double> img_mirror(img_mirror_size.CellCount(), 0);
    auto img_mirror_span = gsl::as_multi_span(img_mirror);
    for (int i = 0; i < image.size.row; i++) {
        for (int j = 0; j < image.size.col; j++) {
            img_mirror_span[i * (image.size.col + 1) + j] = image.Get(i, j);
        }
    }

    // duplicate last row
    for (int j = 0; j < image.size.col + 1; j++) {
        img_mirror_span[image.size.row * (image.size.col + 1) + j] =
              img_mirror_span[(image.size.row - 1) * (image.size.col + 1) + j];
    }

    // duplicate last col
    for (int i = 0; i < image.size.row + 1; i++) {
        img_mirror_span[(i + 1) * (image.size.col + 1) - 1] =
              img_mirror_span[(i + 1) * (image.size.col + 1) - 2];
    }

    // convolve. BLN_kernel is already flipped
    for (int i = 0; i < image.size.row; ++i) {
        for (int j = 0; j < image.size.col; ++j) {
            interpolated_im.Set(
                  i, j,
                  img_mirror_span[i * img_mirror_size.col + j] * BLN_kernel[0] +
                        img_mirror_span[i * img_mirror_size.col + j + 1] *
                              BLN_kernel[1] +
                        img_mirror_span[(i + 1) * img_mirror_size.col + j] *
                              BLN_kernel[2] +
                        img_mirror_span[(i + 1) * img_mirror_size.col + j + 1] *
                              BLN_kernel[3]);
        }
    }

    // use "- shift" to remove borders that should have been replicated on the
    // opposite side of the translation if we had passed the entire shift
    return RemoveBorders(interpolated_im, std::ceil(-row_shift),
                         std::ceil(-col_shift));
}

}  // namespace translation
}  // namespace sirius