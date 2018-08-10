#include "sirius/gdal/resampling/output_stream.h"

#include "sirius/gdal/wrapper.h"

namespace sirius {
namespace gdal {
namespace resampling {

OutputStream::OutputStream(
      const std::string& input_path, const std::string& output_path,
      const sirius::resampling::Parameters& resampling_parameters) {
    auto input_dataset = gdal::LoadDataset(input_path);
    zoom_ratio_ = resampling_parameters.ratio;

    int output_h =
          std::ceil(input_dataset->GetRasterYSize() * zoom_ratio_.ratio());
    int output_w =
          std::ceil(input_dataset->GetRasterXSize() * zoom_ratio_.ratio());

    auto geo_ref =
          ComputeResampledGeoReference(input_path, resampling_parameters.ratio);

    auto output_dataset =
          gdal::CreateDataset(output_path, output_w, output_h, 1, geo_ref);
    LOG("resampled_output_stream", info, "resampled image '{}' ({}x{})",
        output_path, output_h, output_w);

    output_stream_ = gdal::OutputStream(std::move(output_dataset));
}

void OutputStream::Write(StreamBlock&& block, std::error_code& ec) {
    int out_row_idx =
          std::floor(block.row_idx * zoom_ratio_.input_resolution() /
                     static_cast<double>(zoom_ratio_.output_resolution()));
    int out_col_idx =
          std::floor(block.col_idx * zoom_ratio_.input_resolution() /
                     static_cast<double>(zoom_ratio_.output_resolution()));
    LOG("resampled_output_stream", debug,
        "writing block ({},{}) to ({},{}) (size: {}x{})", block.row_idx,
        block.col_idx, out_row_idx, out_col_idx, block.buffer.size.row,
        block.buffer.size.col);

    block.row_idx = out_row_idx;
    block.col_idx = out_col_idx;

    output_stream_.Write(std::move(block), ec);
}

}  // namespace resampling
}  // namespace gdal
}  // namespace sirius