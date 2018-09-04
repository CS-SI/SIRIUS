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
#include <cmath>

#include "sirius/rotation/utils.h"

#include "sirius/utils/log.h"

namespace sirius {
namespace rotation {

Size ComputeNonRotatedHull(const Size& image_size, const int angle) {
    double angle_rad = angle * M_PI / 180.0;

    // get minimal size that contains the rotated image
    int min_width;
    int min_height;
    if (angle < 90) {
        min_width = image_size.col * std::abs(std::cos(angle_rad)) +
                    image_size.row * std::abs(std::sin(angle_rad));
        min_height = image_size.col * std::abs(std::sin(angle_rad)) +
                     image_size.row * std::abs(std::cos(angle_rad));
    } else {
        auto h_tmp = image_size.col;
        auto w_tmp = image_size.row;
        auto angle_tmp_rad = (angle - 90) * M_PI / 180.0;
        min_width = std::round(w_tmp * std::abs(std::cos(angle_tmp_rad)) +
                               h_tmp * std::abs(std::sin(angle_tmp_rad)));
        min_height = std::round(w_tmp * std::abs(std::sin(angle_tmp_rad)) +
                                h_tmp * std::abs(std::cos(angle_tmp_rad)));
    }

    return {min_height, min_width};
}

void RecoverCorners(const Size& image_size, const int angle,
                    const Size& hull_size, Point& tr, Point& tl, Point& br,
                    Point& bl) {
    auto angle_rad = angle * M_PI / 180.0;

    if (angle >= 0) {
        tr = Point(image_size.col * std::cos(angle_rad), 0);
        br = Point(hull_size.col, image_size.row * std::cos(angle_rad));
        tl = Point(0, image_size.col * std::sin(angle_rad));
        bl = Point(image_size.row * std::sin(angle_rad), hull_size.row);
    } else {
        tr = Point(hull_size.col,
                   std::abs(image_size.col * std::sin(angle_rad)));
        br = Point(image_size.col * std::cos(angle_rad), hull_size.row);
        tl = Point(std::abs(image_size.row * std::sin(angle_rad)), 0);
        bl = Point(0, image_size.row * std::cos(angle_rad));
    }

    LOG("RecoverCorners", debug,
        "block corners : tl = {}, {}, tr = {}, {}, bl = {}, {}, br = {}, {}",
        tl.x, tl.y, tr.x, tr.y, bl.x, bl.y, br.x, br.y);
}

}  // namespace rotation
}  // namespace sirius