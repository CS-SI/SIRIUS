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

#ifndef SIRIUS_ZOOM_FREQUENCY_ZOOM_BASE_H_
#define SIRIUS_ZOOM_FREQUENCY_ZOOM_BASE_H_

#include "sirius/i_frequency_zoom.h"
#include "sirius/image.h"

#include "sirius/fftw/types.h"

namespace sirius {
namespace zoom {

/**
 * \brief Implementation of IFrequencyZoom
 */
template <template <class> class ImageDecompositionPolicy, class ZoomStrategy>
class FrequencyZoom final : public IFrequencyZoom,
                            private ImageDecompositionPolicy<ZoomStrategy> {
  public:
    FrequencyZoom() = default;

    // copyable
    FrequencyZoom(const FrequencyZoom&) = default;
    FrequencyZoom& operator=(const FrequencyZoom&) = default;
    // moveable
    FrequencyZoom(FrequencyZoom&&) = default;
    FrequencyZoom& operator=(FrequencyZoom&&) = default;

    ~FrequencyZoom() override = default;

    // IFrequencyZoom interface
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

}  // namespace zoom
}  // namespace sirius

#include "sirius/zoom/frequency_zoom.txx"

#endif  // SIRIUS_FREQ_ZOOM_H_
