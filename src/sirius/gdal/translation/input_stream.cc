#include "sirius/gdal/translation/input_stream.h"

#include "sirius/utils/numeric.h"

namespace sirius {
namespace gdal {
namespace translation {

InputStream::InputStream(
      const std::string& image_path, const sirius::Size& block_size, bool,
      const sirius::translation::Parameters& translation_parameters)
    : input_dataset_(gdal::LoadDataset(image_path)),
      block_size_(block_size),
      is_ended_(false),
      row_idx_(0),
      col_idx_(0),
      row_shift_(translation_parameters.row_shift),
      col_shift_(translation_parameters.col_shift) {
    block_size_.row += std::ceil(std::abs(row_shift_));
    block_size_.col += std::ceil(std::abs(col_shift_));
    if (block_size_.row <= 0 || block_size_.col <= 0) {
        LOG("translation_input_stream", error, "invalid block size");
        throw sirius::Exception("invalid block size");
    }

    LOG("translation_input_stream", info, "input image '{}' ({}x{})",
        image_path, input_dataset_->GetRasterYSize(),
        input_dataset_->GetRasterXSize());
}

StreamBlock InputStream::Read(std::error_code& ec) {
    if (is_ended_) {
        ec = make_error_code(CPLE_ObjectNull);
        return {};
    }

    int w = input_dataset_->GetRasterXSize();
    int h = input_dataset_->GetRasterYSize();
    int w_to_read = block_size_.col;
    int h_to_read = block_size_.row;

    if (w_to_read > w || h_to_read > h) {
        LOG("translation_input_stream", warn,
            "requested block size ({}x{}) is bigger than source image "
            "({}x{}). ",
            w_to_read, h_to_read, w, h);
        w_to_read = w;
        h_to_read = h;
    }

    // resize block if needed
    if (row_idx_ + h_to_read > h) {
        // assign size that can be read
        h_to_read -= (row_idx_ + h_to_read - h);
    }
    if (col_idx_ + w_to_read > w) {
        w_to_read -= (col_idx_ + w_to_read - w);
    }

    Image output_buffer({h_to_read, w_to_read});

    CPLErr err = input_dataset_->GetRasterBand(1)->RasterIO(
          GF_Read, col_idx_, row_idx_, w_to_read, h_to_read,
          output_buffer.data.data(), w_to_read, h_to_read, GDT_Float64, 0, 0);

    if (err) {
        LOG("translation_input_stream", error,
            "GDAL error: {} - could not read from the dataset", err);
        ec = make_error_code(err);
        return {};
    }

    int block_row_idx = row_idx_;
    int block_col_idx = col_idx_;

    StreamBlock output_block(std::move(output_buffer), block_row_idx,
                             block_col_idx, Padding(0, 0, 0, 0));

    if (((row_idx_ + block_size_.row) >= h) &&
        ((col_idx_ + block_size_.col) >= w)) {
        is_ended_ = true;
    }

    if (col_idx_ >= w - block_size_.col) {
        col_idx_ = 0;
        row_idx_ += block_size_.row - std::ceil(std::abs(row_shift_));
    } else {
        col_idx_ += block_size_.col - std::ceil(std::abs(col_shift_));
    }

    LOG("translation_input_stream", debug,
        "reading block of size {}x{} at ({},{})", output_block.buffer.size.row,
        output_block.buffer.size.col, output_block.row_idx,
        output_block.col_idx);

    ec = make_error_code(CPLE_None);

    return output_block;
}
}  // namespace translation
}  // namespace gdal
}  // namespace sirius