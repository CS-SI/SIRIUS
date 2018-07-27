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

#include "sirius/fftw/wrapper.h"

#include <cstring>

#include "sirius/fftw/exception.h"
#include "sirius/fftw/fftw.h"

namespace sirius {
namespace fftw {

ComplexUPtr CreateComplex(const Size& size) {
    ComplexUPtr complex(fftw_alloc_complex(size.CellCount()));
    if (complex == nullptr) {
        LOG("fftw", critical,
            "not enough memory to allocate complex of size {}x{}", size.row,
            size.col);
        throw fftw::Exception(fftw::ErrorCode::kComplexAllocationFailed);
    }
    std::memset(complex.get(), 0, size.CellCount() * sizeof(fftw_complex));
    return complex;
}

RealUPtr CreateReal(const Size& size) {
    RealUPtr real(fftw_alloc_real(size.CellCount()));
    if (real == nullptr) {
        LOG("fftw", critical,
            "not enough memory to allocate complex of size {}x{}", size.row,
            size.col);
        throw fftw::Exception(fftw::ErrorCode::kRealAllocationFailed);
    }

    std::memset(real.get(), 0, size.CellCount() * sizeof(double));
    return real;
}

ComplexUPtr FFT(const Image& image) {
    auto val_real = CreateReal(image.size);
    std::memcpy(val_real.get(), image.data.data(),
                image.size.CellCount() * sizeof(double));

    return FFT(val_real.get(), image.size);
}

ComplexUPtr FFT(double* values, const Size& size) {
    auto fft = fftw::CreateComplex({size.row, size.col / 2 + 1});
    auto fft_plan =
          Fftw::Instance().GetRealToComplexPlan(size, values, fft.get());

    fftw_execute_dft_r2c(fft_plan.get(), values, fft.get());

    return fft;
}

Image IFFT(const Size& image_size, ComplexUPtr image_fft) {
    auto zoomed_values = CreateReal(image_size);

    // fftw expects image_fft of size H*(W/2 +1) and needs output
    // dims to create ifft plan
    auto ifft_plan = Fftw::Instance().GetComplexToRealPlan(
          image_size, image_fft.get(), zoomed_values.get());

    fftw_execute_dft_c2r(ifft_plan.get(), image_fft.get(), zoomed_values.get());

    // store zoomed_values into image
    Image zoomed_image(image_size);
    std::memcpy(zoomed_image.data.data(), zoomed_values.get(),
                zoomed_image.CellCount() * sizeof(double));

    return zoomed_image;
}

}  // namespace fftw
}  // namespace sirius
