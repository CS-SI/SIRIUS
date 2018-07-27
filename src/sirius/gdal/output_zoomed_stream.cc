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

#include "sirius/gdal/output_zoomed_stream.h"

#include "sirius/gdal/error_code.h"
#include "sirius/gdal/wrapper.h"

#include "sirius/utils/log.h"

namespace sirius {
namespace gdal {

OutputZoomedStream::OutputZoomedStream(const std::string& input_path,
                                       const std::string& output_path,
                                       const ZoomRatio& zoom_ratio)
    : zoom_ratio_(zoom_ratio) {
    auto input_dataset = gdal::LoadDataset(input_path);

    int output_h =
          std::ceil(input_dataset->GetRasterYSize() * zoom_ratio_.ratio());
    int output_w =
          std::ceil(input_dataset->GetRasterXSize() * zoom_ratio_.ratio());

    auto geo_ref = gdal::ComputeZoomedGeoReference(input_path, zoom_ratio);
    output_dataset_ =
          gdal::CreateDataset(output_path, output_w, output_h, 1, geo_ref);
    LOG("output_stream", info, "output image \"{}\", size: {}x{}", output_path,
        output_h, output_w);
}

void OutputZoomedStream::Write(StreamBlock&& block, std::error_code& ec) {
    int out_row_idx =
          std::floor(block.row_idx * zoom_ratio_.input_resolution() /
                     static_cast<double>(zoom_ratio_.output_resolution()));
    int out_col_idx =
          std::floor(block.col_idx * zoom_ratio_.input_resolution() /
                     static_cast<double>(zoom_ratio_.output_resolution()));

    LOG("output_stream", debug, "writing {}x{} at {}x{}", block.buffer.size.row,
        block.buffer.size.col, out_row_idx, out_col_idx);

    CPLErr err = output_dataset_->GetRasterBand(1)->RasterIO(
          GF_Write, out_col_idx, out_row_idx, block.buffer.size.col,
          block.buffer.size.row, const_cast<double*>(block.buffer.data.data()),
          block.buffer.size.col, block.buffer.size.row, GDT_Float64, 0, 0,
          NULL);
    if (err) {
        LOG("output_zoomed_stream", error,
            "GDAL error: {} - could not write to the given dataset", err);
        ec = make_error_code(err);
        return;
    }
    ec = make_error_code(CPLE_None);
}

}  // namespace gdal
}  // namespace sirus
