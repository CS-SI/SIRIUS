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

#include "sirius/gdal/debug.h"

#include "sirius/fftw/types.h"

#include "sirius/gdal/wrapper.h"

namespace sirius {
namespace gdal {

#ifdef NDEBUG

void SaveFFTAsImage(const fftw_complex* fft, const Size& image_size,
                    const std::string& output_filepath) {
    Size fft_size(image_size.row, image_size.col / 2 + 1);
    LOG("gdal", trace, "saving fft {}x{} into '{}'", fft_size.row, fft_size.col,
        output_filepath);
    std::vector<double> real_part(fft_size.CellCount());
    std::vector<double> im_part(fft_size.CellCount());
    for (int i = 0; i < fft_size.CellCount(); ++i) {
        real_part[i] = fft[i][0];
        im_part[i] = fft[i][1];
    }

    auto dataset = gdal::CreateDataset(output_filepath, fft_size.col,
                                       fft_size.row, 1, {});
    auto band = dataset->GetRasterBand(1);
    CPLErr err = band->RasterIO(GF_Write, 0, 0, fft_size.col, fft_size.row,
                                real_part.data(), fft_size.col, fft_size.row,
                                GDT_Float64, 0, 0);
    if (err) {
        LOG("image", error, "GDAL error: {} - could not write in file {}", err,
            output_filepath);
    }
}

#else

void SaveFFTAsImage(const fftw_complex*, const Size&, const std::string&) {}

#endif  // NDEBUG

}  // namespace gdal
}  // namespace sirius