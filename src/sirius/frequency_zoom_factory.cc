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

#include "sirius/frequency_zoom_factory.h"

#include "sirius/utils/log.h"

#include "sirius/zoom/frequency_zoom.h"
#include "sirius/zoom/image_decomposition/periodic_smooth_policy.h"
#include "sirius/zoom/image_decomposition/regular_policy.h"
#include "sirius/zoom/zoom_strategy/periodization_strategy.h"
#include "sirius/zoom/zoom_strategy/zero_padding_strategy.h"

namespace sirius {

IFrequencyZoom::UPtr FrequencyZoomFactory::Create(
      ImageDecompositionPolicies image_decomposition,
      FrequencyZoomStrategies zoom_strategy) {
    using FrequencyZoomRegularZeroPadding =
          zoom::FrequencyZoom<zoom::ImageDecompositionRegularPolicy,
                              zoom::ZeroPaddingZoomStrategy>;

    using FrequencyZoomRegularPeriodization =
          zoom::FrequencyZoom<zoom::ImageDecompositionRegularPolicy,
                              zoom::PeriodizationZoomStrategy>;

    using FrequencyZoomPeriodicSmoothZeroPadding =
          zoom::FrequencyZoom<zoom::ImageDecompositionPeriodicSmoothPolicy,
                              zoom::ZeroPaddingZoomStrategy>;

    using FrequencyZoomPeriodicSmoothPeriodization =
          zoom::FrequencyZoom<zoom::ImageDecompositionPeriodicSmoothPolicy,
                              zoom::PeriodizationZoomStrategy>;

    switch (image_decomposition) {
        case ImageDecompositionPolicies::kRegular:
            switch (zoom_strategy) {
                case FrequencyZoomStrategies::kZeroPadding:
                    return std::make_unique<FrequencyZoomRegularZeroPadding>();
                case FrequencyZoomStrategies::kPeriodization:
                    return std::make_unique<
                          FrequencyZoomRegularPeriodization>();
                default:
                    break;
            }
            break;
        case ImageDecompositionPolicies::kPeriodicSmooth:
            switch (zoom_strategy) {
                case FrequencyZoomStrategies::kZeroPadding:
                    return std::make_unique<
                          FrequencyZoomPeriodicSmoothZeroPadding>();
                case FrequencyZoomStrategies::kPeriodization:
                    return std::make_unique<
                          FrequencyZoomPeriodicSmoothPeriodization>();
                default:
                    break;
            }
            break;
        default:
            break;
    }

    LOG("frequency_zoom_factory", warn,
        "combination of image decomposition and zoom strategy not implemented");
    return nullptr;
}

}  // namespace sirius
