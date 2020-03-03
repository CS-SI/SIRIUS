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

#include "sirius/image_decomposition/periodic_smooth_policy.h"
#include "sirius/image_decomposition/regular_policy.h"

#include "sirius/resampling/frequency_resampler.h"
#include "sirius/resampling/upsampling/periodization_upsampling.h"
#include "sirius/resampling/upsampling/zero_padding_upsampling.h"

#include "sirius/utils/log.h"

namespace sirius {

IFrequencyResampler::UPtr FrequencyResamplerFactory::Create(
      image_decomposition::Policies image_decomposition,
      FrequencyUpsamplingStrategies zoom_strategy) {
    using FrequencyResamplerRegularZeroPadding =
          resampling::FrequencyResampler<image_decomposition::RegularPolicy,
                                         resampling::ZeroPaddingUpsampling>;

    using FrequencyResamplerRegularPeriodization =
          resampling::FrequencyResampler<image_decomposition::RegularPolicy,
                                         resampling::PeriodizationUpsampling>;

    using FrequencyResamplerPeriodicSmoothZeroPadding =
          resampling::FrequencyResampler<
                image_decomposition::PeriodicSmoothPolicy,
                resampling::ZeroPaddingUpsampling>;

    using FrequencyResamplerPeriodicSmoothPeriodization =
          resampling::FrequencyResampler<
                image_decomposition::PeriodicSmoothPolicy,
                resampling::PeriodizationUpsampling>;

    switch (image_decomposition) {
        case image_decomposition::Policies::kRegular:
            switch (zoom_strategy) {
                case FrequencyUpsamplingStrategies::kZeroPadding:
                    return std::make_unique<
                          FrequencyResamplerRegularZeroPadding>();
                case FrequencyUpsamplingStrategies::kPeriodization:
                    return std::make_unique<
                          FrequencyResamplerRegularPeriodization>();
                default:
                    break;
            }
            break;
        case image_decomposition::Policies::kPeriodicSmooth:
            switch (zoom_strategy) {
                case FrequencyUpsamplingStrategies::kZeroPadding:
                    return std::make_unique<
                          FrequencyResamplerPeriodicSmoothZeroPadding>();
                case FrequencyUpsamplingStrategies::kPeriodization:
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
        "combination of image decomposition and upsampling strategy not "
        "implemented");
    return nullptr;
}

}  // namespace sirius
