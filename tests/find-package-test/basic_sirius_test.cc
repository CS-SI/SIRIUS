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

#include <sirius/utils/log.h>

#include <sirius/frequency_resampler_factory.h>
#include <sirius/types.h>
#include "sirius/filter.h"
#include "sirius/image.h"

int main(int, char**) {
    LOG_SET_LEVEL(trace);

    auto zoom_ratio_2_1 = sirius::ZoomRatio::Create(2, 1);

    auto dummy_image = sirius::Image({5, 5});

    sirius::IFrequencyResampler::UPtr freq_resampler =
          sirius::FrequencyResamplerFactory::Create(
                sirius::image_decomposition::Policies::kRegular,
                sirius::FrequencyUpsamplingStrategies::kZeroPadding);

    sirius::Image zoomed_image_2_1 =
          freq_resampler->Compute(dummy_image, {}, {zoom_ratio_2_1, nullptr});
}