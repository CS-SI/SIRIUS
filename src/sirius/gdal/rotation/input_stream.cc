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

#include "sirius/gdal/rotation/input_stream.h"

#include "sirius/rotation/utils.h"

#include "sirius/utils/numeric.h"

namespace sirius {
namespace gdal {
namespace rotation {

InputStream::InputStream(
      const std::string& image_path, const sirius::Size& block_size, bool,
      const sirius::rotation::Parameters& rotation_parameters)
    : input_dataset_(gdal::LoadDataset(image_path)),
      block_size_(block_size),
      is_ended_(false),
      row_idx_(0),
      col_idx_(0),
      angle_(rotation_parameters.angle) {
    if (block_size_.row <= 0 || block_size_.col <= 0) {
        LOG("rotation_input_stream", error, "invalid block size");
        throw sirius::Exception("invalid block size");
    }

    LOG("rotation_input_stream", info, "input image '{}' ({}x{})", image_path,
        input_dataset_->GetRasterYSize(), input_dataset_->GetRasterXSize());
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
        LOG("rotation_input_stream", warn,
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
        LOG("rotation_input_stream", error,
            "GDAL error: {} - could not read from the dataset", err);
        ec = make_error_code(err);
        return {};
    }

    ///////
    GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
    GDALDataset* dataset = driver->Create(
          "/home/mbelloc/tmp/read_block.tif", output_buffer.size.col,
          output_buffer.size.row, 1, GDT_Float32, NULL);
    dataset->GetRasterBand(1)->RasterIO(
          GF_Write, 0, 0, output_buffer.size.col, output_buffer.size.row,
          output_buffer.data.data(), output_buffer.size.col,
          output_buffer.size.row, GDT_Float64, 0, 0, NULL);
    GDALClose(dataset);
    ///////

    if (row_idx_ == 0 && col_idx_ == 0) {
        // initialize output indexes to the top left corner of the
        // rotated output image
        Point tr, tl, br, bl;
        auto min_output_size =
              sirius::rotation::ComputeNonRotatedHull({h, w}, angle_);
        sirius::rotation::RecoverCorners({h, w}, angle_, min_output_size, tr,
                                         tl, br, bl);
        block_row_idx_ = tl.y;
        block_col_idx_ = tl.x;
        tl_ref_ = tl;
    } else {
        // update indexes thanks to corners coordinates inside block
        Point tr, tl, br, bl;
        auto min_block_size =
              sirius::rotation::ComputeNonRotatedHull(block_size_, angle_);
        sirius::rotation::RecoverCorners(block_size_, angle_, min_block_size,
                                         tr, tl, br, bl);
        if (angle_ > 0) {
            if (reset_row_) {
                // add BL coordinates to reference top left
                // because BL inside a block should always
                // have the same coordinates for 1 block row
                tl_ref_.x += bl.x;
                tl_ref_.y += bl.y;
                block_row_idx_ = tl_ref_.y;
                block_col_idx_ = tl_ref_.x;
                reset_row_ = false;
            } else {
                block_col_idx_ += tr.x;
                block_row_idx_ -= tl.y;
            }
        } else {
            if (reset_row_) {
                tl_ref_.x -= tl.x;
                tl_ref_.y += bl.y;
                block_row_idx_ = tl_ref_.y;
                block_col_idx_ = tl_ref_.x;
                reset_row_ = false;
            } else {
                block_col_idx_ += tr.x;
                block_row_idx_ += tr.y;
            }
        }
    }

    StreamBlock output_block(std::move(output_buffer), block_row_idx_,
                             block_col_idx_, Padding(0, 0, 0, 0),
                             {h_to_read, w_to_read});

    LOG("rotation_input_stream", debug,
        "reading block of size {}x{} at ({},{})", h_to_read, w_to_read,
        row_idx_, col_idx_);

    if (((row_idx_ + block_size_.row) >= h) &&
        ((col_idx_ + block_size_.col) >= w)) {
        is_ended_ = true;
    }

    if (col_idx_ >= w - block_size_.col) {
        col_idx_ = 0;
        row_idx_ += block_size_.row;
        reset_row_ = true;
    } else {
        col_idx_ += block_size_.col;
    }

    ec = make_error_code(CPLE_None);

    return output_block;
}
}  // namespace rotation
}  // namespace gdal
}  // namespace sirius