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

#ifndef SIRIUS_TRANSLATION_PARAMETERS_H_
#define SIRIUS_TRANSLATION_PARAMETERS_H_

namespace sirius {
namespace translation {

/**
 * \brief Data class that represents translation parameters
 */
struct Parameters {
    Parameters(float i_row_shift, float i_col_shift)
        : row_shift(i_row_shift), col_shift(i_col_shift) {}

    float row_shift;
    float col_shift;
};

}  // namespace resampling
}  // namespace sirius

#endif  // SIRIUS_TRANSLATION_PARAMETERS_H_