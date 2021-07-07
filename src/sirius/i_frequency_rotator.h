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

#ifndef SIRIUS_I_FREQUENCY_ROTATOR_H_
#define SIRIUS_I_FREQUENCY_ROTATOR_H_

#include <memory>

#include "sirius/image.h"

#include "sirius/rotation/parameters.h"

namespace sirius {

/**
 * \class IFrequencyRotation
 * \brief Interface that frequency rotation should implement
 */
class IFrequencyRotator {
  public:
    using UPtr = std::unique_ptr<IFrequencyRotator>;
    using Parameters = rotation::Parameters;

  public:
    virtual ~IFrequencyRotator() = default;

    /**
     * \brief Rotate an image
     *
     * \remark This method is thread safe
     *
     * \param input image to rotate
     * \param image_padding expected padding to add to the image
     * \param parameters rotation parameters
     * \return Rotated image
     *
     * \throw SiriusException if a computing issue happens
     */
    virtual Image Compute(const Image& input, const Padding& image_padding,
                          const Parameters& parameters) const = 0;
};

}  // namespace sirius

#endif  // SIRIUS_I_FREQUENCY_ROTATOR_H_
