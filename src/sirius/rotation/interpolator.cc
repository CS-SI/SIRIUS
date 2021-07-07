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

#include "sirius/rotation/interpolator.h"
#include "sirius/rotation/utils.h"

#include <cmath>
#include <cstring>

#include "sirius/exception.h"

#include "sirius/utils/gsl.h"
#include "sirius/utils/log.h"
#include "sirius/utils/numeric.h"

namespace sirius {
namespace rotation {

// TODO : fix this. on 90° rotation there is a entire column of poluted data
// that is copied and should not
Image Interpolator::Interpolate2D(const Image& image,
                                  const Parameters& parameters) const {
    Point tr, tl, br, bl;
    Size min_size = ComputeNonRotatedHull(image.size, parameters.angle);
    RecoverCorners(image.size, parameters.angle, min_size, tr, tl, br, bl);

    double angle_rad = (parameters.angle * M_PI) / 180.0;
    double cos_angle = std::cos(angle_rad);
    double sin_angle = std::sin(angle_rad);

    Image output_image(min_size);
    Point center(image.size.col / 2, image.size.row / 2);
    Point center_out(min_size.col / 2, min_size.row / 2);

    std::vector<double> BLN_kernel(4);
    BLN_kernel[0] = cos_angle;
    BLN_kernel[1] = sin_angle;
    BLN_kernel[2] = -sin_angle;
    BLN_kernel[3] = cos_angle;

    for (int x = 0; x < min_size.col; ++x) {
        for (int y = 0; y < min_size.row; ++y) {
            Point p(x - center_out.x, y - center_out.y);
            Point p1(p.x * BLN_kernel[0] + p.y * BLN_kernel[2],
                     p.x * BLN_kernel[1] + p.y * BLN_kernel[3]);
            p1.x += center.x;
            p1.y += center.y;
            if (p1.x >= 0 && p1.x < image.size.col && p1.y >= 0 &&
                p1.y < image.size.row) {
                output_image.data[y * min_size.col + x] =
                      image.data[p1.y * image.size.col + p1.x];
            }
        }
    }
    return output_image;
}

}  // namespace rotation
}  // namespace sirius