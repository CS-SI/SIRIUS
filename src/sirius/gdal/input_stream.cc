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

#include "sirius/gdal/input_stream.h"

#include "sirius/exception.h"
#include "sirius/types.h"

#include "sirius/gdal/error_code.h"
#include "sirius/gdal/wrapper.h"

#include "sirius/utils/log.h"

namespace sirius {
namespace gdal {

InputStream::InputStream(const std::string& image_path,
                         const sirius::Size& block_size,
                         const sirius::Size& block_margin_size,
                         PaddingType block_padding_type)
    : input_dataset_(gdal::LoadDataset(image_path)),
      block_size_(block_size),
      block_margin_size_(block_margin_size),
      block_padding_type_(block_padding_type),
      is_ended_(false),
      row_idx_(0),
      col_idx_(0) {
    if (block_size_.row <= 0 || block_size_.col <= 0) {
        LOG("input_stream", error, "invalid block size");
        throw sirius::Exception("invalid block size");
    }

    LOG("input_stream", info, "input image '{}' ({}x{})", image_path,
        input_dataset_->GetRasterYSize(), input_dataset_->GetRasterXSize());
}

StreamBlock InputStream::Read(std::error_code& ec) {
    if (is_ended_) {
        ec = make_error_code(CPLE_ObjectNull);
        return {};
    }

    int w = input_dataset_->GetRasterXSize();
    int h = input_dataset_->GetRasterYSize();
    int padded_block_w = block_size_.col + 2 * block_margin_size_.col;
    int padded_block_h = block_size_.row + 2 * block_margin_size_.row;

    if (padded_block_w > w || padded_block_h > h) {
        LOG("input_stream", critical,
            "requested block size ({}x{}) is bigger than source image ({}x{}). "
            "You should use regular processing",
            padded_block_w, padded_block_h, w, h);
        ec = make_error_code(CPLE_ObjectNull);
        return {};
    }

    // resize block if needed
    if (row_idx_ + padded_block_h > h) {
        // assign size that can be read
        padded_block_h -= (row_idx_ + padded_block_h - h);

        if (padded_block_h < block_margin_size_.row) {
            LOG("input_stream", error,
                "block at coordinates ({}, {}) cannot be read because "
                "available reading height {} is less than margin size {}",
                row_idx_, col_idx_, padded_block_h, block_margin_size_.row);
            ec = make_error_code(CPLE_ObjectNull);
            return {};
        }

        if (padded_block_h > block_margin_size_.row + block_size_.row) {
            // bottom margin is partly read. add missing margin
            padded_block_h +=
                  (row_idx_ + block_size_.row + 2 * block_margin_size_.row - h);
        } else {
            padded_block_h += block_margin_size_.row;
        }
    }
    if (col_idx_ + padded_block_w > w) {
        padded_block_w -= (col_idx_ + padded_block_w - w);

        if (padded_block_w < block_margin_size_.col) {
            LOG("ReadBlock", error,
                "block at coordinates {}, {}, cannot be read because available "
                "reading width {} is less than margin size {}",
                row_idx_, col_idx_, padded_block_w, block_margin_size_.col);
            ec = make_error_code(CPLE_ObjectNull);
            return {};
        }

        if (padded_block_w > block_size_.col + block_margin_size_.col) {
            padded_block_w +=
                  (col_idx_ + block_size_.col + 2 * block_margin_size_.col - w);
        } else {
            padded_block_w += block_margin_size_.col;
        }
    }
    Padding block_padding;
    block_padding.type = block_padding_type_;
    int w_to_read = padded_block_w;
    int h_to_read = padded_block_h;
    // top padding needed
    if (row_idx_ == 0) {
        block_padding.top = block_margin_size_.row;
        h_to_read -= block_margin_size_.row;
    }

    // bottom padding needed
    if (row_idx_ >= (h - block_size_.row - 2 * block_margin_size_.row)) {
        block_padding.bottom = block_margin_size_.row;
        h_to_read -= (row_idx_ + padded_block_h - h);
    }

    // left padding needed
    if (col_idx_ == 0) {
        block_padding.left = block_margin_size_.col;
        w_to_read -= block_margin_size_.col;
    }

    // right padding needed
    if (col_idx_ >= (w - block_size_.col - 2 * block_margin_size_.col)) {
        block_padding.right = block_margin_size_.col;
        w_to_read -= (col_idx_ + padded_block_w - w);
    }

    Image output_buffer({h_to_read, w_to_read});

    CPLErr err = input_dataset_->GetRasterBand(1)->RasterIO(
          GF_Read, col_idx_, row_idx_, w_to_read, h_to_read,
          output_buffer.data.data(), w_to_read, h_to_read, GDT_Float64, 0, 0);

    if (err) {
        LOG("input_stream", error,
            "GDAL error: {} - could not read from the dataset", err);
        ec = make_error_code(err);
        return {};
    }

    int block_row_idx = (row_idx_ == 0) ? 0 : row_idx_ + block_margin_size_.row;
    int block_col_idx = (col_idx_ == 0) ? 0 : col_idx_ + block_margin_size_.col;

    StreamBlock output_block(std::move(output_buffer), block_row_idx,
                             block_col_idx, block_padding);

    if (((row_idx_ + padded_block_h - block_margin_size_.row) >= h) &&
        ((col_idx_ + padded_block_w - block_margin_size_.col) >= w)) {
        is_ended_ = true;
    }

    if (col_idx_ >= w - block_size_.col - block_margin_size_.col) {
        col_idx_ = 0;
        if (row_idx_ == 0) {
            row_idx_ += block_size_.row - block_margin_size_.row;
        } else {
            row_idx_ += block_size_.row;
        }
    } else {
        if (col_idx_ == 0) {
            col_idx_ += block_size_.col - block_margin_size_.col;
        } else {
            col_idx_ += block_size_.col;
        }
    }

    LOG("input_stream", debug, "reading block of size {}x{} at ({},{})",
        output_block.buffer.size.row, output_block.buffer.size.col,
        output_block.row_idx, output_block.col_idx);

    ec = make_error_code(CPLE_None);
    return output_block;
}

}  // namespace gdal
}  // namespace sirius
