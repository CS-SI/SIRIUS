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

#ifndef SIRIUS_RESAMPLER_IMAGE_DECOMPOSITION_REGULAR_POLICY_TXX_
#define SIRIUS_RESAMPLER_IMAGE_DECOMPOSITION_REGULAR_POLICY_TXX_

#include "sirius/resampler/image_decomposition/regular_policy.h"

namespace sirius {
namespace resampler {

template <class ZoomStrategy>
Image ImageDecompositionRegularPolicy<ZoomStrategy>::DecomposeAndZoom(
      int zoom, const Image& padded_image, const Filter& filter) const {
    // method inherited from ZoomStrategy
    LOG("regular_decomposition", trace, "zoom image");
    return this->Zoom(zoom, padded_image, filter);
}

}  // namespace resampler
}  // namespace sirius

#endif  // SIRIUS_RESAMPLER_IMAGE_DECOMPOSITION_REGULAR_POLICY_TXX_
