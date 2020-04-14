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

#ifndef SIRIUS_FREQUENCY_ROTATOR_H_
#define SIRIUS_FREQUENCY_ROTATOR_H_

#include "sirius/i_frequency_rotator.h"

#include "sirius/rotation/interpolator.h"
#include "sirius/rotation/processor.h"

namespace sirius {
namespace rotation {

/**
 * \brief Implementation of IFrequencyRotator
 */
template <template <class, class, class> class ImageDecompositionPolicy>
class FrequencyRotator final
      : public IFrequencyRotator,
        private ImageDecompositionPolicy<IFrequencyRotator, rotation::Processor,
                                         rotation::Interpolator> {
  public:
    FrequencyRotator() = default;

    ~FrequencyRotator() = default;
    // copyable
    FrequencyRotator(const FrequencyRotator&) = default;
    FrequencyRotator& operator=(const FrequencyRotator&) = default;
    // moveable
    FrequencyRotator(FrequencyRotator&&) = default;
    FrequencyRotator& operator=(FrequencyRotator&&) = default;

    // IFrequencyResampler interface
    Image Compute(const Image& input, const Padding&,
                  const Parameters& rotation_parameters) const override {
        return this->DecomposeAndProcess(input, rotation_parameters);
    }
};

}  // namespace rotation
}  // namespace sirius

#endif  // SIRIUS_FREQUENCY_ROTATOR_H_
