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

#ifndef SIRIUS_TRANSLATION_UTILS_H_
#define SIRIUS_TRANSLATION_UTILS_H_

#include "sirius/image.h"

namespace sirius {
namespace translation {

/**
  \brief remove borders according to given shifts.
  \param row_shift shift on y axis
  \param col_shift shift on x axis
  \return cropped image
 */
Image RemoveBorders(const Image& image, int row_shift, int col_shift);

}  // namespace translation
}  // namespace sirius

#endif  // SIRIUS_TRANSLATION_UTILS_H_