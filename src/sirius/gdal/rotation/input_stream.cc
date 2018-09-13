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
#include "sirius/gdal/input_stream.h"

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
        LOG("rotation_input_stream", error, "invalid block size {}x{}",
            block_size_.row, block_size_.col);
        throw sirius::Exception("invalid block size");
    }

    if (block_size_.row > input_dataset_->GetRasterYSize() ||
        block_size_.col > input_dataset_->GetRasterXSize()) {
        const_cast<Size&>(block_size_).row = input_dataset_->GetRasterYSize();
        const_cast<Size&>(block_size_).col = input_dataset_->GetRasterXSize();
    }

    // used to know when we have to use previous block size to update indexes
    blocks_per_band_ = std::ceil(input_dataset_->GetRasterXSize() /
                                 static_cast<float>(block_size_.col));
    nb_bands_ = std::ceil(input_dataset_->GetRasterYSize() /
                          static_cast<float>(block_size_.row));

    LOG("rotation_input_stream", info, "input image '{}' ({}x{})", image_path,
        input_dataset_->GetRasterYSize(), input_dataset_->GetRasterXSize());
    LOG("rotation_input_stream", info, "block size = {}x{}", block_size_.row,
        block_size_.col);
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
        LOG("rotation_input_stream", warn,
            "requested block size ({}x{}) is bigger than source image ({}x{}). "
            "Read only one block",
            padded_block_w, padded_block_h, w, h);
        padded_block_h = h + 2 * block_margin_size_.row;
        padded_block_w = w + 2 * block_margin_size_.col;
    } else {
        // resize block if needed
        if (row_idx_ + padded_block_h > h) {
            // assign size that can be read
            padded_block_h -= (row_idx_ + padded_block_h - h);

            if (padded_block_h < block_margin_size_.row) {
                LOG("rotation_input_stream", error,
                    "block at coordinates ({}, {}) cannot be read because "
                    "available reading height {} is less than margin size {}",
                    row_idx_, col_idx_, padded_block_h, block_margin_size_.row);
                ec = make_error_code(CPLE_ObjectNull);
                return {};
            }

            if (padded_block_h > block_margin_size_.row + block_size_.row) {
                // bottom margin is partly read. add missing margin
                padded_block_h += (row_idx_ + block_size_.row +
                                   2 * block_margin_size_.row - h);
            } else {
                padded_block_h += block_margin_size_.row;
            }
        }
        if (col_idx_ + padded_block_w > w) {
            padded_block_w -= (col_idx_ + padded_block_w - w);

            if (padded_block_w < block_margin_size_.col) {
                LOG("rotation_input_stream", error,
                    "block at coordinates {}, {}, cannot be read because "
                    "available "
                    "reading width {} is less than margin size {}",
                    row_idx_, col_idx_, padded_block_w, block_margin_size_.col);
                ec = make_error_code(CPLE_ObjectNull);
                return {};
            }

            if (padded_block_w > block_size_.col + block_margin_size_.col) {
                padded_block_w += (col_idx_ + block_size_.col +
                                   2 * block_margin_size_.col - w);
            } else {
                padded_block_w += block_margin_size_.col;
            }
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
    if (row_idx_ >= (h - h_to_read)) {
        block_padding.bottom = block_margin_size_.row;
        h_to_read -= (row_idx_ + h_to_read - h);
    }

    // left padding needed
    if (col_idx_ == 0) {
        block_padding.left = block_margin_size_.col;
        w_to_read -= block_margin_size_.col;
    }

    // right padding needed
    if (col_idx_ >= (w - w_to_read)) {
        block_padding.right = block_margin_size_.col;
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

    // recover size without margins
    Size src_size(h_to_read, w_to_read);
    if (block_padding.top == 0 ||
        (block_padding.top > 0 && block_padding.top < block_margin_size_.row)) {
        src_size.row -= (block_margin_size_.row - block_padding.top);
    }
    if (block_padding.bottom == 0 ||
        (block_padding.bottom > 0 &&
         block_padding.bottom < block_margin_size_.row)) {
        src_size.row -= (block_margin_size_.row - block_padding.bottom);
    }
    if (block_padding.left == 0 ||
        (block_padding.left > 0 &&
         block_padding.left < block_margin_size_.col)) {
        src_size.col -= (block_margin_size_.col - block_padding.left);
    }
    if (block_padding.right == 0 ||
        (block_padding.right > 0 &&
         block_padding.right < block_margin_size_.col)) {
        src_size.col -= (block_margin_size_.col - block_padding.right);
    }

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
        // use previous corners if we are at the end of a band because we need
        // to use previous block size to shift output indexes
        if (block_count_ == blocks_per_band_ - 1 ||
            band_count_ == nb_bands_ - 1) {
            tr = tr_prev_;
            tl = tl_prev_;
            br = br_prev_;
            bl = bl_prev_;
        } else {
            auto min_block_size =
                  sirius::rotation::ComputeNonRotatedHull(src_size, angle_);
            sirius::rotation::RecoverCorners(src_size, angle_, min_block_size,
                                             tr, tl, br, bl);
        }

        // block indexes must be increased / decreased by corners coordinates
        if (angle_ > 0) {
            if (reset_row_) {
                // shift reference top left to band's bottom left (new TL)
                tl_ref_.x += bl.x;
                tl_ref_.y += bl.y - tl.y;
                block_row_idx_ = tl_ref_.y;
                block_col_idx_ = tl_ref_.x;
                reset_row_ = false;
                block_count_ = 0;
            } else {
                // shift next block indexes to current block's top right corner
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
                block_count_ = 0;
            } else {
                block_col_idx_ += tr.x - tl.x;
                block_row_idx_ += tr.y;
                // fix 1 pixel shift between blocks
                if (angle_ > -80 && angle_ < -90) {
                    block_col_idx_++;
                }
            }
        }
        if (block_count_ != blocks_per_band_ - 1 &&
            band_count_ != nb_bands_ - 1) {
            tl_prev_ = tl;
            tr_prev_ = tr;
            bl_prev_ = bl;
            br_prev_ = br;
        }
    }

    StreamBlock output_block(std::move(output_buffer), block_row_idx_,
                             block_col_idx_, block_padding, src_size,
                             {h_to_read, w_to_read});

    LOG("rotation_input_stream", debug,
        "reading block of size {}x{} at ({},{})", output_block.buffer.size.row,
        output_block.buffer.size.col, col_idx_, row_idx_);

    if (((row_idx_ + padded_block_h - block_margin_size_.row) >= h) &&
        ((col_idx_ + padded_block_w - block_margin_size_.col) >= w)) {
        is_ended_ = true;
    }

    // update reading indexes
    if (col_idx_ >= w - block_size_.col - block_margin_size_.col) {
        reset_row_ = true;
        band_count_++;
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
    block_count_++;

    ec = make_error_code(CPLE_None);
    return output_block;
}

}  // namespace rotation
}  // namespace gdal
}  // namespace sirius