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

#ifndef SIRIUS_RESAMPLER_ZOOM_STRATEGY_PERIODIZATION_STRATEGY_H_
#define SIRIUS_RESAMPLER_ZOOM_STRATEGY_PERIODIZATION_STRATEGY_H_

#include "sirius/fftw/types.h"
#include "sirius/filter.h"
#include "sirius/image.h"

namespace sirius {
namespace resampler {

/**
 * \brief Implementation of periodization frequency zoom
 */
class PeriodizationZoomStrategy {
  public:
    Image Zoom(int zoom, const Image& padded_image, const Filter& filter) const;

  private:
    fftw::ComplexUPtr PeriodizeFFT(int zoom, const Image& image,
                                   fftw::ComplexUPtr image_fft) const;
};

}  // namespace resampler
}  // namespace sirius

#endif  // SIRIUS_RESAMPLER_ZOOM_STRATEGY_PERIODIZATION_STRATEGY_H_
