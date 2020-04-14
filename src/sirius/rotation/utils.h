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

#ifndef SIRIUS_ROTATION_UTILS_H_
#define SIRIUS_ROTATION_UTILS_H_

#include "sirius/image.h"
#include "sirius/types.h"

namespace sirius {
namespace rotation {

/**
 * \brief compute the minimal rectangle that contains the rotated image
 * \brief image_size input image size
 * \brief angle rotation angle
 * \return minimal size
 */
Size ComputeNonRotatedHull(const Size& image_size, const int angle);

/**
 * \brief recover coordinates of the hull's corners
 * \param image_size input image size
 * \param angle rotation angle
 * \param hull_size dimensions of the hull
 * \param tr output top right corner
 * \param tl output top left corner
 * \param br output bottom right corner
 * \param bl output bl corner
 */
void RecoverCorners(const Size& image_size, const int angle,
                    const Size& hull_size, Point& tr, Point& tl, Point& br,
                    Point& bl);

}  // namespace rotation
}  // namespace sirius

#endif  // SIRIUS_ROTATION_UTILS_H_