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

#include "sirius/frequency_resampler_factory.h"

#include "sirius/utils/log.h"

#include "sirius/resampler/frequency_resampler.h"
#include "sirius/resampler/image_decomposition/periodic_smooth_policy.h"
#include "sirius/resampler/image_decomposition/regular_policy.h"
#include "sirius/resampler/zoom_strategy/periodization_strategy.h"
#include "sirius/resampler/zoom_strategy/zero_padding_strategy.h"

namespace sirius {

IFrequencyResampler::UPtr FrequencyResamplerFactory::Create(
      ImageDecompositionPolicies image_decomposition,
      FrequencyZoomStrategies zoom_strategy) {
    using FrequencyResamplerRegularZeroPadding = resampler::FrequencyResampler<
          resampler::ImageDecompositionRegularPolicy,
          resampler::ZeroPaddingZoomStrategy>;

    using FrequencyResamplerRegularPeriodization =
          resampler::FrequencyResampler<
                resampler::ImageDecompositionRegularPolicy,
                resampler::PeriodizationZoomStrategy>;

    using FrequencyResamplerPeriodicSmoothZeroPadding =
          resampler::FrequencyResampler<
                resampler::ImageDecompositionPeriodicSmoothPolicy,
                resampler::ZeroPaddingZoomStrategy>;

    using FrequencyResamplerPeriodicSmoothPeriodization =
          resampler::FrequencyResampler<
                resampler::ImageDecompositionPeriodicSmoothPolicy,
                resampler::PeriodizationZoomStrategy>;

    switch (image_decomposition) {
        case ImageDecompositionPolicies::kRegular:
            switch (zoom_strategy) {
                case FrequencyZoomStrategies::kZeroPadding:
                    return std::make_unique<
                          FrequencyResamplerRegularZeroPadding>();
                case FrequencyZoomStrategies::kPeriodization:
                    return std::make_unique<
                          FrequencyResamplerRegularPeriodization>();
                default:
                    break;
            }
            break;
        case ImageDecompositionPolicies::kPeriodicSmooth:
            switch (zoom_strategy) {
                case FrequencyZoomStrategies::kZeroPadding:
                    return std::make_unique<
                          FrequencyResamplerPeriodicSmoothZeroPadding>();
                case FrequencyZoomStrategies::kPeriodization:
                    return std::make_unique<
                          FrequencyResamplerPeriodicSmoothPeriodization>();
                default:
                    break;
            }
            break;
        default:
            break;
    }

    LOG("frequency_resampler_factory", warn,
        "combination of image decomposition and zoom strategy not implemented");
    return nullptr;
}

}  // namespace sirius
