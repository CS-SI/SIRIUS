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

#include "sirius/frequency_translator_factory.h"

#include "sirius/utils/log.h"

#include "sirius/image_decomposition/periodic_smooth_policy.h"
#include "sirius/image_decomposition/regular_policy.h"
#include "sirius/translation/frequency_translator.h"

namespace sirius {

IFrequencyTranslator::UPtr FrequencyTranslatorFactory::Create(
      image_decomposition::Policies image_decomposition) {
    using FrequencyTranslatorRegular =
          translation::FrequencyTranslator<image_decomposition::RegularPolicy>;

    using FrequencyTranslatorPeriodicSmooth = translation::FrequencyTranslator<
          image_decomposition::PeriodicSmoothPolicy>;

    switch (image_decomposition) {
        case image_decomposition::Policies::kRegular:
            return std::make_unique<FrequencyTranslatorRegular>();
        case image_decomposition::Policies::kPeriodicSmooth:
            return std::make_unique<FrequencyTranslatorPeriodicSmooth>();
            break;
        default:
            break;
    }

    LOG("frequency_translator_factory", warn, "image decomposition");
    return nullptr;
}

}  // namespace sirius
