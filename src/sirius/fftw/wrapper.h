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

#ifndef SIRIUS_FFTW_WRAPPER_H_
#define SIRIUS_FFTW_WRAPPER_H_

#include <gsl/gsl>

#include "sirius/image.h"
#include "sirius/types.h"

#include "sirius/fftw/types.h"

namespace sirius {
namespace fftw {

/**
 * \brief Create complex array and initialize it to 0
 * \param size complex array size
 * \return fftw_complex unique ptr
 * \throws sirius::fftw::Exception if the complex creation fails
 */
ComplexUPtr CreateComplex(const Size& size);

/**
 * \brief Create real array and initialize it to 0
 * \param size real array size
 * \return double* unique ptr
 * \throws sirius::fftw::Exception if the real creation fails
 */
RealUPtr CreateReal(const Size& size);

/**
 * \brief Compute the FFT of an image
 * \param image input image
 * \return complex array unique ptr
 * \throws sirius::fftw::Exception if the computation of FFT failed
 */
ComplexUPtr FFT(const Image& image);

/**
 * \brief Compute the FFT of real array
 * \param values initialized values
 * \param size values size
 * \return complex array unique ptr
 * \throws sirius::fftw::Exception if the computation of FFT failed
 */
ComplexUPtr FFT(double* values, const Size& size);

/**
 * \brief Compute the IFFT of an image FFT
 * \param image_size image size
 * \param image_fft image FFT
 * \return image
 * \throws sirius::fftw::Exception if the computation of IFFT failed
 */
Image IFFT(const Size& image_size, ComplexUPtr image_fft);

}  // namespace fftw
}  // namespace sirius

#endif  // SIRIUS_FFTW_WRAPPER_H_
