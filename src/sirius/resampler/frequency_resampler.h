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

#ifndef SIRIUS_RESAMPLER_FREQUENCY_RESAMPLER_H_
#define SIRIUS_RESAMPLER_FREQUENCY_RESAMPLER_H_

#include "sirius/i_frequency_resampler.h"
#include "sirius/image.h"

#include "sirius/fftw/types.h"

namespace sirius {
namespace resampler {

/**
 * \brief Implementation of IFrequencyResampler
 */
template <template <class> class ImageDecompositionPolicy, class ZoomStrategy>
class FrequencyResampler final
      : public IFrequencyResampler,
        private ImageDecompositionPolicy<ZoomStrategy> {
  public:
    FrequencyResampler() = default;

    // copyable
    FrequencyResampler(const FrequencyResampler&) = default;
    FrequencyResampler& operator=(const FrequencyResampler&) = default;
    // moveable
    FrequencyResampler(FrequencyResampler&&) = default;
    FrequencyResampler& operator=(FrequencyResampler&&) = default;

    ~FrequencyResampler() override = default;

    // IFrequencyResampler interface
    Image Compute(const ZoomRatio& ratio, const Image& input,
                  const Padding& image_padding,
                  const Filter& filter = {}) const override;

  private:
    Image UnpadImage(const ZoomRatio& zoom_ratio, const Image& original_image,
                     const Image& zoomed_image, const Padding& image_padding,
                     const Filter& filter) const;

    Image DecimateImage(const Image& zoomed_image,
                        const ZoomRatio& zoom_ratio) const;
};

}  // namespace resampler
}  // namespace sirius

#include "sirius/resampler/frequency_resampler.txx"

#endif  // SIRIUS_RESAMPLER_FREQUENCY_RESAMPLER_H_
