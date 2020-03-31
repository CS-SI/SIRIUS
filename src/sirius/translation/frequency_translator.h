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

#ifndef SIRIUS_FREQUENCY_TRANSLATOR_H_
#define SIRIUS_FREQUENCY_TRANSLATOR_H_

#include "sirius/i_frequency_translator.h"
#include "sirius/translation/interpolator.h"
#include "sirius/translation/processor.h"

namespace sirius {
namespace translation {

/**
 * \brief Implementation of IFrequencyTranslator
 */
template <template <class, class, class> class ImageDecompositionPolicy>
class FrequencyTranslator final
      : public IFrequencyTranslator,
        private ImageDecompositionPolicy<IFrequencyTranslator,
                                         translation::Processor,
                                         translation::Interpolator> {
  public:
    FrequencyTranslator() = default;

    ~FrequencyTranslator() = default;
    // copyable
    FrequencyTranslator(const FrequencyTranslator&) = default;
    FrequencyTranslator& operator=(const FrequencyTranslator&) = default;
    // moveable
    FrequencyTranslator(FrequencyTranslator&&) = default;
    FrequencyTranslator& operator=(FrequencyTranslator&&) = default;

    // IFrequencyResampler interface
    Image Compute(const Image& input, const Padding&,
                  const Parameters& translation_parameters) const override;
};

}  // namespace translation
}  // namespace sirius

#include "sirius/translation/frequency_translator.txx"

#endif  // SIRIUS_FREQUENCY_TRANSLATOR_H_
