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

#include "sirius/gdal/translation/output_stream.h"

#include "sirius/gdal/wrapper.h"

#include "sirius/utils/log.h"

namespace sirius {
namespace gdal {
namespace translation {

OutputStream::OutputStream(
      const std::string& input_path, const std::string& output_path,
      const sirius::translation::Parameters& translation_parameters) {
    auto input_dataset = gdal::LoadDataset(input_path);

    int output_h = input_dataset->GetRasterYSize() -
                   std::ceil(std::abs(translation_parameters.row_shift));
    int output_w = input_dataset->GetRasterXSize() -
                   std::ceil(std::abs(translation_parameters.col_shift));

    auto geo_ref = gdal::ComputeShiftedGeoReference(
          input_path, translation_parameters.row_shift,
          translation_parameters.col_shift);
    auto output_dataset =
          gdal::CreateDataset(output_path, output_w, output_h, 1, geo_ref);

    output_stream_ = gdal::OutputStream(std::move(output_dataset));

    LOG("translation_output_stream", info, "translated image '{}' ({}x{})",
        output_path, output_h, output_w);
}

void OutputStream::Write(StreamBlock&& block, std::error_code& ec) {
    output_stream_.Write(std::move(block), ec);
}

}  // namespace translation
}  // namespace gdal
}  // namespace sirius