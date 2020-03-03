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

#ifndef SIRIUS_I_FREQUENCY_RESAMPLER_H_
#define SIRIUS_I_FREQUENCY_RESAMPLER_H_

#include <memory>
#include <string>
#include <vector>

#include "sirius/image.h"

#include "sirius/resampling/parameters.h"

namespace sirius {

/**
 * \class IFrequencyResampler
 * \brief Interface that frequency resampler should implement
 */
class IFrequencyResampler {
  public:
    using UPtr = std::unique_ptr<IFrequencyResampler>;
    using Parameters = resampling::Parameters;

  public:
    virtual ~IFrequencyResampler() = default;

    /**
     * \brief Resample an image by a ratio in the frequency domain
     *
     * \remark This method is thread safe
     *
     * \param input image to zoom in/out
     * \param image_padding expected padding to add to the image to
     *        comply with the filter
     * \param parameters resampling parameters
     * \return Resampled image
     *
     * \throw sirius::Exception if a computing issue happens
     */
    virtual Image Compute(const Image& input, const Padding& image_padding,
                          const Parameters& resampling_parameters) const = 0;
};

}  // namespace sirius

#endif  // SIRIUS_I_FREQUENCY_RESAMPLER_H_
