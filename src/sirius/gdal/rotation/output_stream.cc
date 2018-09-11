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

#include "sirius/gdal/rotation/output_stream.h"
#include "sirius/gdal/error_code.h"
#include "sirius/gdal/wrapper.h"

#include "sirius/rotation/utils.h"

#include "sirius/utils/log.h"
#include "sirius/utils/numeric.h"

namespace sirius {
namespace gdal {
namespace rotation {

OutputStream::OutputStream(
      const std::string& input_path, const std::string& output_path,
      const sirius::rotation::Parameters& rotation_parameters)
    : angle_(rotation_parameters.angle) {
    auto input_dataset = gdal::LoadDataset(input_path);

    auto output_size = sirius::rotation::ComputeNonRotatedHull(
          {input_dataset->GetRasterYSize(), input_dataset->GetRasterXSize()},
          angle_);

    Point tr, tl, br, bl;
    sirius::rotation::RecoverCorners(
          {input_dataset->GetRasterYSize(), input_dataset->GetRasterXSize()},
          angle_, output_size, tr, tl, br, bl);

    // process slopes between each points. Used to know how many pixels are
    // added when we write line by line
    if (angle_ > 0) {
        slope_tr_br_ = (br.y - tr.y) / static_cast<double>(br.x - tr.x);
        slope_tr_tl_ = (tl.y - tr.y) / static_cast<double>(tl.x - tr.x);
        slope_br_bl_ = (bl.y - br.y) / static_cast<double>(bl.x - br.x);
        slope_tl_bl_ = (bl.y - tl.y) / static_cast<double>(bl.x - tl.x);
    } else {
        slope_tl_tr_ = (tr.y - tl.y) / static_cast<double>(tr.x - tl.x);
        slope_tl_bl_ = (bl.y - tl.y) / static_cast<double>(bl.x - tl.x);
        slope_tr_br_ = (br.y - tr.y) / static_cast<double>(br.x - tr.x);
        slope_bl_br_ = (br.y - bl.y) / static_cast<double>(br.x - bl.x);
    }

    auto output_dataset_uptr =
          gdal::CreateDataset(output_path, output_size.col, output_size.row, 1,
                              GetGeoReference(input_dataset.get()));
    output_dataset_ = output_dataset_uptr.get();

    output_stream_ = gdal::OutputStream(std::move(output_dataset_uptr));

    LOG("rotation_output_stream", info, "rotated image '{}' ({}x{})",
        output_path, output_size.row, output_size.col);
}

void OutputStream::Write(StreamBlock&& block, std::error_code& ec) {
    LOG("rotation_output_stream", debug,
        "writing block of size {}x{} at ({}, {})", block.buffer.size.row,
        block.buffer.size.col, block.col_idx, block.row_idx);

    Point tr_src, tl_src, br_src, bl_src, tr, tl, br, bl;
    Size hull_original_size =
          sirius::rotation::ComputeNonRotatedHull(block.original_size, angle_);
    sirius::rotation::RecoverCorners(block.original_size, angle_,
                                     hull_original_size, tr_src, tl_src, br_src,
                                     bl_src);
    sirius::rotation::RecoverCorners(block.original_size_with_margins, angle_,
                                     block.buffer.size, tr, tl, br, bl);

    // process vectors between each source points
    if (angle_ < 0) {
        tl_tr_vector_ = {tr_src.y - tl_src.y, tr_src.x - tl_src.x};
        tr_br_vector_ = {br_src.y - tr_src.y, br_src.x - tr_src.x};
        tl_bl_vector_ = {bl_src.y - tl_src.y, bl_src.x - tl_src.x};
        bl_br_vector_ = {br_src.y - bl_src.y, br_src.x - bl_src.x};
    } else {
        tr_br_vector_ = {br_src.y - tr_src.y, br_src.x - tr_src.x};
        br_bl_vector_ = {bl_src.y - br_src.y, bl_src.x - br_src.x};
        tr_tl_vector_ = {tl_src.y - tr_src.y, tl_src.x - tr_src.x};
        tl_bl_vector_ = {bl_src.y - tl_src.y, bl_src.x - tl_src.x};
    }

    Point begin_marged_block(block.col_idx, block.row_idx);
    CopyConvexHull(block, begin_marged_block, tr, tl, br, bl);
    ec = make_error_code(CPLE_None);
}

void OutputStream::CopyConvexHull(const sirius::gdal::StreamBlock& block,
                                  Point& begin_marged_block, const Point& tr,
                                  const Point& tl, const Point& br,
                                  const Point& bl) {
    LOG("rotation_output_stream", trace,
        "Copy convex hull to output image, hull_size = {}x{}, row_idx = "
        "{}, col_idx = {}",
        block.buffer.size.row, block.buffer.size.col, begin_marged_block.x,
        begin_marged_block.y);

    double col_idx_real = begin_marged_block.x;

    if (angle_ == 90) {
        // height and width are swapped because of 90° rotation so we add
        // height dim to x shift and width dim to y shift
        int begin_y = begin_marged_block.y - block.original_size.col;
        int begin_block_x = block_margin_size_.row - block.padding.top;
        int begin_block_y = block_margin_size_.col - block.padding.right;

        for (int i = begin_y; i < begin_y + block.original_size.col; ++i) {
            sirius::gdal::WriteToDataset(
                  output_dataset_, i, begin_marged_block.x, 1,
                  block.original_size.row,
                  block.buffer.data.data() +
                        begin_block_y * block.buffer.size.col + begin_block_x);
            begin_block_y++;
        }

        return;
    }

    if (angle_ == -90) {
        // height and width are swapped because of -90° rotation so we add
        // height dim to x shift and width dim to y shift
        int begin_x = begin_marged_block.x - block.original_size.row;
        int begin_block_x = block_margin_size_.row - block.padding.bottom + 1;
        int begin_block_y = block_margin_size_.col - block.padding.left;
        for (int i = begin_marged_block.y;
             i < begin_marged_block.y + block.original_size.col; ++i) {
            sirius::gdal::WriteToDataset(
                  output_dataset_, i, begin_x, 1, block.original_size.row,
                  block.buffer.data.data() +
                        begin_block_y * block.buffer.size.col + begin_block_x);
            begin_block_y++;
        }
        return;
    }

    if (angle_ >= 0) {
        Point tr_src = tr;

        // process shifts needed to recover source block (without margins)
        // inside the block with margins
        if (block.padding.top != block_margin_size_.row) {
            // margin may not be completely read
            int read_margin = block_margin_size_.row - block.padding.top;
            double angle_rad = (90 - angle_) * M_PI / 180.0;
            int shift_x = std::round(read_margin * std::cos(angle_rad));
            int shift_y = std::round(read_margin * std::sin(angle_rad));
            tr_src.x += shift_x;
            tr_src.y += shift_y;
            // shift the "global" point where we'll start to write in the final
            // image
            if (block.padding.bottom == block_margin_size_.row) {
                begin_marged_block.x -= shift_x;
                begin_marged_block.y += shift_y;
            }
        }

        int shift_x = 0;
        int shift_y = 0;
        if (block.padding.right != block_margin_size_.col) {
            int read_margin = block_margin_size_.col - block.padding.right;
            double angle_rad = angle_ * M_PI / 180.0;
            shift_x = std::round(read_margin * std::cos(angle_rad));
            shift_y = std::round(read_margin * std::sin(angle_rad));
            tr_src.x -= shift_x;
            tr_src.y += shift_y;
            begin_marged_block.y += shift_y;
        }

        Point tl_src(tr_src.x + tr_tl_vector_.col,
                     tr_src.y + tr_tl_vector_.row);
        Point br_src(tr_src.x + tr_br_vector_.col,
                     tr_src.y + tr_br_vector_.row);
        Point bl_src(tl_src.x + tl_bl_vector_.col,
                     tl_src.y + tl_bl_vector_.row);

        begin_marged_block.x += tr_src.x;
        begin_marged_block.y -= tl_src.y;
        col_idx_real = begin_marged_block.x;
        if (block.padding.left != block_margin_size_.col) {
            int read_margin = block_margin_size_.col - block.padding.left;
            double angle_rad = angle_ * M_PI / 180.0;
            shift_x = std::round(read_margin * std::cos(angle_rad));
            begin_marged_block.x -= shift_x;
            col_idx_real = begin_marged_block.x;
        }

        if (block.padding.bottom != block_margin_size_.row &&
            block.padding.top != block_margin_size_.row) {
            int read_margin = block_margin_size_.row - block.padding.bottom;
            double angle_rad = (90 - angle_) * M_PI / 180.0;
            shift_x = std::round(read_margin * std::cos(angle_rad));
            shift_y = std::round(read_margin * std::sin(angle_rad));
            begin_marged_block.x -= shift_x;
            col_idx_real = begin_marged_block.x;
            begin_marged_block.y += shift_y;
        }

        // first and second points used to detect when we have to change slopes
        Point first_point;
        Point second_point;

        if (br.y <= tl.y) {
            first_point = br_src;
            second_point = tl_src;
        } else {
            first_point = tl_src;
            second_point = br_src;
        }

        int begin_line = tr_src.y * block.buffer.size.col + tr_src.x +
                         std::ceil(1 / slope_tr_tl_);
        int end_line = tr_src.y * block.buffer.size.col + tr_src.x +
                       std::ceil(1 / slope_tr_br_);

        double begin_line_real = begin_line;
        double end_line_real = end_line;
        double slope_begin = slope_tr_tl_;
        double slope_end = slope_tr_br_;
        // copy from the point with coordinate y=0 to the first point of the
        // hull
        CopyHullPart(block, begin_line, end_line, begin_line_real,
                     end_line_real, begin_marged_block.y, begin_marged_block.x,
                     col_idx_real, tr_src.y, first_point.y, slope_begin,
                     slope_end);

        // updates slopes that allows us to know when to start and stop
        // copying a line
        if (br.y <= tl.y) {
            slope_end = slope_br_bl_;
        } else {
            slope_begin = slope_tl_bl_;
        }

        // copy from the first point with coordinate y != 0 to the second
        // point of the hull
        CopyHullPart(block, begin_line, end_line, begin_line_real,
                     end_line_real, begin_marged_block.y, begin_marged_block.x,
                     col_idx_real, first_point.y, second_point.y, slope_begin,
                     slope_end);

        // update slopes
        if (br.y <= tl.y) {
            slope_begin = slope_tl_bl_;
        } else {
            slope_end = slope_br_bl_;
        }

        // copy from the second point we encountered until we reach source point
        // with the highest height
        CopyHullPart(block, begin_line, end_line, begin_line_real,
                     end_line_real, begin_marged_block.y, begin_marged_block.x,
                     col_idx_real, second_point.y, bl_src.y, slope_begin,
                     slope_end);
    } else {
        Point tl_src = tl;

        // process shifts needed to recover source block (without margins)
        // inside the block with margins
        if (block.padding.top != block_margin_size_.row) {
            // margin may not be completely read
            int read_margin = block_margin_size_.row - block.padding.top;
            double angle_rad = (90 - std::abs(angle_)) * M_PI / 180.0;
            int shift_x = std::round(read_margin * std::cos(angle_rad));
            int shift_y = std::round(read_margin * std::sin(angle_rad));
            tl_src.x -= shift_x;
            tl_src.y += shift_y;
        }

        if (block.padding.left != block_margin_size_.col) {
            int read_margin = block_margin_size_.col - block.padding.left;
            double angle_rad = std::abs(angle_) * M_PI / 180.0;
            int shift_x = std::round(read_margin * std::cos(angle_rad));
            int shift_y = std::round(read_margin * std::sin(angle_rad));
            tl_src.x += shift_x;
            tl_src.y += shift_y;
        }

        Point tr_src(tl_src.x + tl_tr_vector_.col,
                     tl_src.y + tl_tr_vector_.row);
        Point bl_src(tl_src.x + tl_bl_vector_.col,
                     tl_src.y + tl_bl_vector_.row);
        Point br_src(tr_src.x + tr_br_vector_.col,
                     tr_src.y + tr_br_vector_.row);

        Point first_point;
        Point second_point;

        if (bl.y <= tr.y) {
            first_point = bl_src;
            second_point = tr_src;
        } else {
            first_point = tr_src;
            second_point = bl_src;
        }

        // according to rotation angle, top corner may be represented by
        // multiple pixels, so we add slopes
        int begin_line = tl_src.y * block.buffer.size.col + tl_src.x +
                         std::ceil(1 / slope_tl_bl_);
        int end_line = tl_src.y * block.buffer.size.col + tl_src.x +
                       std::ceil(1 / slope_tl_tr_);

        double begin_line_real = begin_line;
        double end_line_real = end_line;
        double slope_begin = slope_tl_bl_;
        double slope_end = slope_tl_tr_;

        // copy from the point with coordinate y=0 to the first point of the
        // hull
        CopyHullPart(block, begin_line, end_line, begin_line_real,
                     end_line_real, begin_marged_block.y, begin_marged_block.x,
                     col_idx_real, tl_src.y, first_point.y, slope_begin,
                     slope_end);

        // updates slopes that allows us to know when to start and stop
        // reading
        if (bl.y <= tr.y) {
            slope_begin = slope_bl_br_;
        } else {
            slope_end = slope_tr_br_;
        }

        // copy from the first point with coordinate y != 0 to the second
        // point of the hull
        CopyHullPart(block, begin_line, end_line, begin_line_real,
                     end_line_real, begin_marged_block.y, begin_marged_block.x,
                     col_idx_real, first_point.y, second_point.y, slope_begin,
                     slope_end);

        // update slopes
        if (bl.y <= tr.y) {
            slope_end = slope_tr_br_;
        } else {
            slope_begin = slope_bl_br_;
        }

        // copy from the second point we encountered until we reach source point
        // with the highest height
        CopyHullPart(block, begin_line, end_line, begin_line_real,
                     end_line_real, begin_marged_block.y, begin_marged_block.x,
                     col_idx_real, second_point.y, br_src.y, slope_begin,
                     slope_end);
    }
}

void OutputStream::CopyHullPart(const sirius::gdal::StreamBlock& block,
                                int& begin_line, int& end_line,
                                double& begin_line_real, double& end_line_real,
                                int& row_idx, int& col_idx,
                                double& col_idx_real, int first_y, int second_y,
                                double slope_begin, double slope_end) {
    int end_it = second_y;
    // avoid out of bounds error
    if (row_idx + (second_y - first_y) > output_dataset_->GetRasterYSize()) {
        end_it = first_y + (output_dataset_->GetRasterYSize() - row_idx);
    }

    for (int i = first_y; i < end_it; ++i) {
        if (col_idx < 0) {
            col_idx = 0;
            col_idx_real = 0;
        }

        int line_len = end_line - begin_line;
        if (line_len > block.buffer.size.col) {
            line_len = block.buffer.size.col;
        }
        if (col_idx + line_len > output_dataset_->GetRasterXSize()) {
            line_len = output_dataset_->GetRasterXSize() - col_idx;
        }

        if (row_idx >= 0) {
            sirius::gdal::WriteToDataset(output_dataset_, row_idx, col_idx, 1,
                                         line_len,
                                         block.buffer.data.data() + begin_line);
        }

        if (angle_ >= 45 ||
            // update indexes thanks to slopes
            (i != second_y - 1 && (angle_ >= -90 && angle_ < 45))) {
            begin_line_real += block.buffer.size.col + 1 / slope_begin;
            end_line_real += block.buffer.size.col + 1 / slope_end;
            begin_line = std::round(begin_line_real);
            end_line = std::round(end_line_real);
            row_idx++;
            col_idx_real += 1 / slope_begin;
            col_idx = std::round(col_idx_real);
        } else {
            row_idx++;
            // experimental (1/3 only viable for 256x256 block size).
            // the row at index sencond_y does not contain 1/slope_begin more
            // pixels than the line before
            col_idx_real += 1 / (3 * slope_begin);
            col_idx = std::round(col_idx_real);
            begin_line_real += block.buffer.size.col + 1 / (3 * slope_begin);
            end_line_real += block.buffer.size.col + 1 / slope_end;
            begin_line = std::round(begin_line_real);
            end_line = std::round(end_line_real);
        }
    }
}

}  // namespace rotation
}  // namespace gdal
}  // namespace sirius