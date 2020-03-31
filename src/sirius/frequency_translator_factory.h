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

#ifndef SIRIUS_FREQUENCY_TRANSLATOR_FACTORY_H_
#define SIRIUS_FREQUENCY_TRANSLATOR_FACTORY_H_

#include "sirius/i_frequency_translator.h"

#include "sirius/image_decomposition/policies.h"

namespace sirius {

/**
 * \brief Factory of IFrequencyTranslation
 */
class FrequencyTranslatorFactory {
  public:
    /**
     * \brief IFrequencyTranslation factory
     * \param image_decomposition cf. image_decomposition::Policies enum
     * \return requested composition of IFrequencyTranslation | nullptr if not
     * available
     */
    static IFrequencyTranslator::UPtr Create(
          image_decomposition::Policies image_decomposition);
};

}  // namespace sirius

#endif  // SIRIUS_FREQUENCY_TRANSLATOR_FACTORY_H_
