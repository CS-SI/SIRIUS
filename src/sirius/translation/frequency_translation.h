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

#ifndef SIRIUS_TRANSLATION_FREQUENCY_TRANSLATION_H_
#define SIRIUS_TRANSLATION_FREQUENCY_TRANSLATION_H_

#include "sirius/image.h"
#include "sirius/types.h"

namespace sirius {

class FrequencyTranslation {
  public:
    FrequencyTranslation() = default;

    ~FrequencyTranslation() = default;

    // non copyable
    FrequencyTranslation(const FrequencyTranslation&) = delete;
    FrequencyTranslation& operator=(const FrequencyTranslation&) = delete;
    // moveable
    FrequencyTranslation(FrequencyTranslation&&) = default;
    FrequencyTranslation& operator=(FrequencyTranslation&&) = default;

    Image Shift(const Image& image, float shift_col, float shift_row);

  private:
    Image RemoveBorders(const Image& image, int shift_col, int shift_row);
};

}  // end namespace sirius

#endif  // SIRIUS_TRANSLATION_FREQUENCY_TRANSLATION_H_