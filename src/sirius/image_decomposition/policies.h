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

#ifndef SIRIUS_IMAGE_DECOMPOSITION_POLICIES_H_
#define SIRIUS_IMAGE_DECOMPOSITION_POLICIES_H_

#include "sirius/filter.h"
#include "sirius/image.h"

namespace sirius {
namespace image_decomposition {

/**
 * \brief Enum of supported image decomposition policies
 */
enum class Policies {
    kRegular = 0,   /**< regular image decomposition */
    kPeriodicSmooth /**< periodic plus smooth image decomposition */
};

}  // namespace image_decomposition
}  // namespace sirius

#endif  // SIRIUS_IMAGE_DECOMPOSITION_POLICIES_H_
