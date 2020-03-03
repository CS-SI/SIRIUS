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

#include "sirius/resampling/interpolator.h"

#include "sirius/utils/gsl.h"

namespace sirius {
namespace resampling {

Image Interpolator::Interpolate2D(const Image& image,
                                  const Parameters& parameters) const {
    int zoom = parameters.ratio.input_resolution();

    Image interpolated_im(image.size * zoom);

    std::vector<double> BLN_kernel(4, 0);
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

    for (int fx = 0; fx < zoom; fx++) {
        for (int fy = 0; fy < zoom; fy++) {
            BLN_kernel[0] = (1 - fx / static_cast<double>(zoom)) *
                            (1 - fy / static_cast<double>(zoom));
            BLN_kernel[1] = (1 - fx / static_cast<double>(zoom)) *
                            (fy / static_cast<double>(zoom));
            BLN_kernel[2] = (fx / static_cast<double>(zoom)) *
                            (1 - fy / static_cast<double>(zoom));
            BLN_kernel[3] = (fx / static_cast<double>(zoom)) *
                            (fy / static_cast<double>(zoom));

            // convolve. BLN_kernel is already flipped
            for (int i = fx; i < image.size.row * zoom; i += zoom) {
                for (int j = fy; j < image.size.col * zoom; j += zoom) {
                    interpolated_im.Set(
                          i, j,
                          img_mirror_span[(i / zoom) * (image.size.col + 1) +
                                          (j / zoom)] *
                                      BLN_kernel[0] +
                                img_mirror_span[(i / zoom + 1) *
                                                      (image.size.col + 1) +
                                                (j / zoom)] *
                                      BLN_kernel[2] +
                                img_mirror_span[(i / zoom) *
                                                      (image.size.col + 1) +
                                                (j / zoom + 1)] *
                                      BLN_kernel[1] +
                                img_mirror_span[(i / zoom + 1) *
                                                      (image.size.col + 1) +
                                                (j / zoom + 1)] *
                                      BLN_kernel[3]);
                }
            }
        }
    }

    return interpolated_im;
}

}  // namespace resampling
}  // namespace sirius