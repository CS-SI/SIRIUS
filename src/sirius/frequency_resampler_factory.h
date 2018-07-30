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

#ifndef SIRIUS_FREQUENCY_RESAMPLER_FACTORY_H_
#define SIRIUS_FREQUENCY_RESAMPLER_FACTORY_H_

#include "sirius/i_frequency_resampler.h"

namespace sirius {

/**
 * \brief Enum of supported image decomposition policies
 */
enum class ImageDecompositionPolicies {
    kRegular = 0,   /**< regular image decomposition */
    kPeriodicSmooth /**< periodic plus smooth image decomposition */
};

/**
 * \brief Enum of supported frequency zoom strategies
 */
enum class FrequencyZoomStrategies {
    kZeroPadding = 0, /**< zero padding zoom */
    kPeriodization    /**< periodization zoom */
};

/**
 * \brief Factory of IFrequencyResampler that composes an image decomposition
 *   policy and a zoom strategy
 */
class FrequencyResamplerFactory {
  public:
    /**
     * \brief IFrequencyResampler factory
     * \param image_decomposition cf. ImageDecompositionPolicies enum
     * \param zoom_strategy cf. FrequencyZoomStrategies enum
     * \return requested composition of FrequencyResampler | nullptr if not
     * available
     */
    static IFrequencyResampler::UPtr Create(
          ImageDecompositionPolicies image_decomposition,
          FrequencyZoomStrategies zoom_strategy);
};

}  // namespace sirius

#endif  // SIRIUS_FREQUENCY_RESAMPLER_FACTORY_H_
