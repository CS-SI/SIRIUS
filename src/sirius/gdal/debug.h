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

#ifndef SIRIUS_GDAL_DEBUG_H_
#define SIRIUS_GDAL_DEBUG_H_

#include <string>

#include <fftw3.h>

#include "sirius/types.h"

namespace sirius {
namespace gdal {

void SaveFFTAsImage(const fftw_complex* fft, const Size& image_size,
                    const std::string& output_filepath);

}  // gdal
}  // sirius

#endif  // SIRIUS_GDAL_DEBUG_H_