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

#ifndef SIRIUS_TESTS_UTILS_H_
#define SIRIUS_TESTS_UTILS_H_

#include "sirius/image.h"
#include "sirius/types.h"

namespace sirius {
namespace tests {

// filter paths
static constexpr char kDiracFilterPath[] = "./filters/dirac_filter.tif";
static constexpr char kSincZoom2FilterPath[] =
      "./filters/sinc_zoom2_filter.tif";

// image paths
static constexpr char kLenaImagePath[] = "./input/lena.jpg";

// output path
static constexpr char kOutputDirectoryPath[] = "./output";

sirius::Image CreateDummyImage(const sirius::Size& size);

sirius::Image CreateSquaredImage(const sirius::Size& size);

}  // namespace tests
}  // namespace sirius

#endif  // SIRIUS_TESTS_UTILS_H_
