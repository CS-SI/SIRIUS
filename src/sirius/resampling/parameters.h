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

#ifndef SIRIUS_RESAMPLING_PARAMETERS_H_
#define SIRIUS_RESAMPLING_PARAMETERS_H_

#include "sirius/filter.h"
#include "sirius/types.h"

namespace sirius {
namespace resampling {

/**
 * \brief Data class that represents resampling parameters
 *
 * Parameters instance is not responsible of the lifetime of the filter. User
 * must insure that the filter will remain valid during lifetime of Parameters
 * instance.
 */
struct Parameters {
    Parameters(const ZoomRatio& i_zoom_ratio)
        : ratio(i_zoom_ratio), filter(nullptr) {}

    Parameters(const ZoomRatio& i_zoom_ratio, const Filter* i_filter)
        : ratio(i_zoom_ratio), filter(i_filter) {}

    ZoomRatio ratio;
    const Filter* filter{nullptr};
};

}  // namespace resampling
}  // namespace sirius

#endif  // SIRIUS_RESAMPLING_PARAMETERS_H_