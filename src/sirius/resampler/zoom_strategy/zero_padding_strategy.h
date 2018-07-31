/**
 * Copyright (C) 2018 CS - Systeme d'Information (CS-SI)
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

#ifndef SIRIUS_ZOOM_ZOOM_STRATEGY_ZERO_PADDING_STRATEGY_H_
#define SIRIUS_ZOOM_ZOOM_STRATEGY_ZERO_PADDING_STRATEGY_H_

#include "sirius/filter.h"
#include "sirius/image.h"

#include "sirius/fftw/types.h"

namespace sirius {
namespace resampler {

/**
 * \brief Implementation of zero padding frequency zoom
 */
class ZeroPaddingZoomStrategy {
  public:
    Image Zoom(int zoom, const Image& padded_image, const Filter& filter) const;

  private:
    fftw::ComplexUPtr ZeroPadFFT(int zoom, const Image& image,
                                 fftw::ComplexUPtr image_fft) const;
};

}  // namespace resampler
}  // namespace sirius

#endif  // SIRIUS_ZOOM_ZOOM_STRATEGY_ZERO_PADDING_STRATEGY_H_
