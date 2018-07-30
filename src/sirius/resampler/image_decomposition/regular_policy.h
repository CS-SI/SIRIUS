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

#ifndef SIRIUS_RESAMPLER_IMAGE_DECOMPOSITION_REGULAR_POLICY_H_
#define SIRIUS_RESAMPLER_IMAGE_DECOMPOSITION_REGULAR_POLICY_H_

#include "sirius/filter.h"
#include "sirius/image.h"

namespace sirius {
namespace resampler {

/**
 * \brief Implementation of regular image decomposition
 */
template <class ZoomStrategy>
class ImageDecompositionRegularPolicy : private ZoomStrategy {
  public:
    Image DecomposeAndZoom(int zoom, const Image& padded_image,
                           const Filter& filter) const;
};

}  // namespace resampler
}  // namespace sirius

#include "sirius/resampler/image_decomposition/regular_policy.txx"

#endif  // SIRIUS_RESAMPLER_IMAGE_DECOMPOSITION_REGULAR_POLICY_H_
