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

#include "sirius/types.h"

#include <cmath>

#include "sirius/exception.h"

#include "sirius/utils/log.h"
#include "sirius/utils/numeric.h"

namespace sirius {

Size::Size(int row, int col) : row(row), col(col) {}

Size::Size(const std::array<int, 2>& size) : row(size[0]), col(size[1]) {}

ZoomRatio ZoomRatio::Create(const std::string& ratio_string) {
    auto delim_pos = ratio_string.find(':');

    if (delim_pos == 0 || delim_pos == ratio_string.size() - 1) {
        LOG("zoom_ratio", error, "invalid ratio string {}", ratio_string);
        throw sirius::Exception("invalid ratio string");
    }

    int input_resolution = 1;
    int output_resolution = 1;
    try {
        if (delim_pos == std::string::npos) {
            // input resolution only
            input_resolution = std::stoi(ratio_string);
        } else {
            input_resolution = std::stoi(ratio_string.substr(0, delim_pos));
            output_resolution = std::stoi(ratio_string.substr(delim_pos + 1));
        }
    } catch (const std::exception& e) {
        LOG("zoom_ratio", error, "invalid ratio string {} ({})", ratio_string,
            e.what());
        throw Exception("invalid ratio string");
    }

    return Create(input_resolution, output_resolution);
}

ZoomRatio ZoomRatio::Create(int input_resolution, int output_resolution) {
    ZoomRatio ratio(input_resolution, output_resolution);
    ratio.ReduceRatio();

    return ratio;
}

ZoomRatio::ZoomRatio(int input_resolution, int output_resolution)
    : input_resolution_(input_resolution),
      output_resolution_(output_resolution) {}

void ZoomRatio::ReduceRatio() {
    if (input_resolution_ <= 0 || output_resolution_ <= 0) {
        LOG("zoom_ratio", error, "invalid zoom ratio {}/{}", input_resolution_,
            output_resolution_);
        throw Exception("invalid zoom ratio");
    }

    // reduce fraction
    int gcd = utils::Gcd(input_resolution_, output_resolution_);

    input_resolution_ /= gcd;
    output_resolution_ /= gcd;
}

bool ZoomRatio::IsRealZoom() const {
    return !(input_resolution_ / output_resolution_ ==
                   std::ceil(input_resolution_ /
                             static_cast<double>(output_resolution_)) &&
             input_resolution_ / output_resolution_ ==
                   std::floor(input_resolution_ /
                              static_cast<double>(output_resolution_)));
}

}  // namespace sirius
