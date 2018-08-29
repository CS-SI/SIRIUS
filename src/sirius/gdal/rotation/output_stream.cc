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

    LOG("OutputStream", debug, "TL x = {}, y = {}", tl.x, tl.y);
    LOG("OutputStream", debug, "TR = {}, y = {}", tr.x, tr.y);
    LOG("OutputStream", debug, "BL x = {}, y = {}", bl.x, bl.y);
    LOG("OutputStream", debug, "BR x = {}, y = {}", br.x, br.y);

    if (angle_ > 0) {
        slope_tr_br_ = (br.y - tr.y) / static_cast<double>(br.x - tr.x);
        slope_tr_tl_ = (tl.y - tr.y) / static_cast<double>(tl.x - tr.x);
        slope_br_bl_ = (bl.y - br.y) / static_cast<double>(bl.x - br.x);
        slope_tl_bl_ = (bl.y - tl.y) / static_cast<double>(bl.x - tl.x);
        LOG("OutputStream", debug, "slope_tr_br = {}", slope_tr_br_);
        LOG("OutputStream", debug, "slope_tr_tl = {}", slope_tr_tl_);
        LOG("OutputStream", debug, "slope_br_bl = {}", slope_br_bl_);
        LOG("OutputStream", debug, "slope_tl_bl = {}", slope_tl_bl_);
    } else {
        slope_tl_tr_ = (tr.y - tl.y) / static_cast<double>(tr.x - tl.x);
        slope_tl_bl_ = (bl.y - tl.y) / static_cast<double>(bl.x - tl.x);
        slope_tr_br_ = (br.y - tr.y) / static_cast<double>(br.x - tr.x);
        slope_bl_br_ = (br.y - bl.y) / static_cast<double>(br.x - bl.x);
        LOG("OutputStream", debug, "slope_tl_tr = {}", slope_tl_tr_);
        LOG("OutputStream", debug, "slope_tl_bl = {}", slope_tl_bl_);
        LOG("OutputStream", debug, "slope_tr_br = {}", slope_tr_br_);
        LOG("OutputStream", debug, "slope_bl_br = {}", slope_bl_br_);
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
        block.buffer.size.col, block.row_idx, block.col_idx);

    Point tr, tl, br, bl;
    sirius::rotation::RecoverCorners(block.original_size, angle_,
                                     block.buffer.size, tr, tl, br, bl);

    LOG("OutputStream", debug,
        "block corners : tl = {}, {}, tr = {}, {}, bl = {}, {}, br = {}, {}",
        tl.x, tl.y, tr.x, tr.y, bl.x, bl.y, br.x, br.y);

    /////
    /*GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
    GDALDataset* dataset = driver->Create(
          "/home/mbelloc/tmp/test_block.tif", block.buffer.size.col,
          block.buffer.size.row, 1, GDT_Float32, NULL);
    dataset->GetRasterBand(1)->RasterIO(
          GF_Write, 0, 0, block.buffer.size.col, block.buffer.size.row,
          block.buffer.data.data(), block.buffer.size.col,
          block.buffer.size.row, GDT_Float64, 0, 0, NULL);
    GDALClose(dataset);*/
    ////////

    CopyConvexHull(block, tr, tl, br, bl);

    ec = make_error_code(CPLE_None);
}

void OutputStream::CopyConvexHull(const sirius::gdal::StreamBlock& block,
                                  const Point& tr, const Point& tl,
                                  const Point& br, const Point& bl) {
    LOG("rotation_output_stream", trace,
        "Copy convex hull to output image, hull_size = {}x{}, row_idx = "
        "{}, col_idx = {}",
        block.buffer.size.row, block.buffer.size.col, block.row_idx,
        block.col_idx);

    int row_idx = block.row_idx;
    int col_idx = block.col_idx;
    double col_idx_real = col_idx;

    if (angle_ == 90) {
        for (int i = block.buffer.size.row - 1; i >= 0; --i) {
            row_idx--;
            gdal::WriteToDataset(
                  output_dataset_, row_idx, col_idx, 1, block.buffer.size.col,
                  block.buffer.data.data() + i * block.buffer.size.col);
        }
        return;
    }

    if (angle_ == -90) {
        for (int i = 0; i < block.buffer.size.row; ++i) {
            sirius::gdal::WriteToDataset(
                  output_dataset_, row_idx, col_idx - block.buffer.size.col, 1,
                  block.buffer.size.col,
                  block.buffer.data.data() + i * block.buffer.size.col);

            row_idx++;
        }
        return;
    }

    if (angle_ >= 0) {
        // block row_idx and col_idx are set to the top left corner, but we
        // start copying from top right corner when angle > 0 so we have to
        // shift it to the right corner
        row_idx -= tl.y;
        col_idx += tr.x;
        col_idx_real += tr.x;
        // first and second points used to detect when we have to change slopes
        Point first_point;
        Point second_point;
        int begin_line = 0;
        int end_line = 0;
        if (angle_ >= 45) {
            begin_line = tr.x;
            end_line = tr.x + 1 / slope_tr_br_;
        } else {
            begin_line = tr.x + 1 / slope_tr_tl_;
            end_line = tr.x;
        }

        if (br.y <= tl.y) {
            first_point = br;
            second_point = tl;
        } else {
            first_point = tl;
            second_point = br;
        }

        LOG("OutputStream", debug, "begin line = {}, end line = {}", begin_line,
            end_line);

        LOG("OutputStream", debug,
            "first point x = {}, y = {}, second point x = {}, y = {}",
            first_point.x, first_point.y, second_point.x, second_point.y);

        double begin_line_real = begin_line;
        double end_line_real = end_line;
        double slope_begin = slope_tr_tl_;
        double slope_end = slope_tr_br_;
        // copy from the point with coordinate y=0 to the first point of the
        // hull
        // LOG("OutputStream", info, "Copy first part");
        CopyHullPart(block, begin_line, end_line, begin_line_real,
                     end_line_real, row_idx, col_idx, col_idx_real, 0,
                     first_point.y, slope_begin, slope_end);

        // updates slopes that allows us to know when to start and stop
        // copying a line
        if (br.y <= tl.y) {
            slope_end = slope_br_bl_;
        } else {
            slope_begin = slope_tl_bl_;
        }

        // copy from the first point with coordinate y != 0 to the second
        // point of the hull
        // LOG("OutputStream", info, "Copy second part");
        CopyHullPart(block, begin_line, end_line, begin_line_real,
                     end_line_real, row_idx, col_idx, col_idx_real,
                     first_point.y, second_point.y, slope_begin, slope_end);

        // update slopes
        if (br.y <= tl.y) {
            slope_begin = slope_tl_bl_;
        } else {
            slope_end = slope_br_bl_;
        }

        // copy from the second point we encountered until we reach image's
        // height
        // LOG("OutputStream", info, "Copy third part");
        CopyHullPart(block, begin_line, end_line, begin_line_real,
                     end_line_real, row_idx, col_idx, col_idx_real,
                     second_point.y, block.buffer.size.row, slope_begin,
                     slope_end);
    } else {
        LOG("OutputStream", debug, "slope_tl_tr = {}", slope_tl_tr_);
        LOG("OutputStream", debug, "slope_tl_bl = {}", slope_tl_bl_);
        LOG("OutputStream", debug, "slope_tr_br = {}", slope_tr_br_);
        LOG("OutputStream", debug, "slope_bl_br = {}", slope_bl_br_);

        Point first_point;
        Point second_point;

        if (bl.y <= tr.y) {
            first_point = bl;
            second_point = tr;
        } else {
            first_point = tr;
            second_point = bl;
        }

        int begin_line = tl.x + 1 / slope_tl_bl_ + 1;
        int end_line = tl.x;
        double begin_line_real = begin_line;
        double end_line_real = end_line;
        double slope_begin = slope_tl_bl_;
        double slope_end = slope_tl_tr_;

        LOG("OutputStream", debug, "begin line = {}, end line = {}", begin_line,
            end_line);

        LOG("OutputStream", debug,
            "first point x = {}, y = {}, second point x = {}, y = {}",
            first_point.x, first_point.y, second_point.x, second_point.y);

        // copy from the point with coordinate y=0 to the first point of the
        // hull
        // LOG("OutputStream", info, "Copy first part");
        CopyHullPart(block, begin_line, end_line, begin_line_real,
                     end_line_real, row_idx, col_idx, col_idx_real, 0,
                     first_point.y, slope_begin, slope_end);

        // updates slopes that allows us to know when to start and stop
        // reading
        if (bl.y <= tr.y) {
            LOG("OutputStream", debug, "slope begin = slope_bl_br_ = {}",
                slope_bl_br_);
            slope_begin = slope_bl_br_;
        } else {
            LOG("OutputStream", debug, "slope end = slope_tr_br_ = {}",
                slope_tr_br_);
            slope_end = slope_tr_br_;
        }

        // copy from the first point with coordinate y != 0 to the second
        // point of the hull
        // LOG("OutputStream", info, "Copy second part");
        CopyHullPart(block, begin_line, end_line, begin_line_real,
                     end_line_real, row_idx, col_idx, col_idx_real,
                     first_point.y, second_point.y, slope_begin, slope_end);

        // update slopes
        if (bl.y <= tr.y) {
            LOG("OutputStream", debug, "slope end = slope_tr_br_ = {}",
                slope_tr_br_);
            slope_end = slope_tr_br_;
        } else {
            LOG("OutputStream", debug, "slope begin = slope_bl_br_ = {}",
                slope_bl_br_);
            slope_begin = slope_bl_br_;
        }

        // copy from the second point we encountered until we reach image's
        // height
        // LOG("OutputStream", info, "Copy third part");
        CopyHullPart(block, begin_line, end_line, begin_line_real,
                     end_line_real, row_idx, col_idx, col_idx_real,
                     second_point.y, block.buffer.size.row, slope_begin,
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
    if (row_idx + (second_y - first_y) > output_dataset_->GetRasterYSize()) {
        end_it = first_y + (output_dataset_->GetRasterYSize() - row_idx);
        LOG("OutputStream", debug,
            "row_idx OOB, row_idx = {}, first_y = {}, second_y = {}, "
            "RasterYSize = {}, end_it = {}",
            row_idx, first_y, second_y, output_dataset_->GetRasterYSize(),
            end_it);
    }

    for (int i = first_y; i < end_it; ++i) {
        if (col_idx < 0) {
            LOG("OutputStream", debug, "col_idx < 0 ");
            col_idx = 0;
            col_idx_real = 0;
        }

        int line_len = end_line - begin_line;
        if (line_len > block.buffer.size.col) {
            LOG("OutputStream", debug, "line len > block.buffer.size.col");
            line_len = block.buffer.size.col;
        }
        if (col_idx + line_len > output_dataset_->GetRasterXSize()) {
            LOG("OutputStream", debug, "line len > output_dataset_ width");
            line_len = output_dataset_->GetRasterXSize() - col_idx;
        }

        LOG("OutputStream", debug, "row_idx = {}, col_idx = {}", row_idx,
            col_idx);
        LOG("OutputStream", debug, "line_len = {}", line_len);

        sirius::gdal::WriteToDataset(output_dataset_, row_idx, col_idx, 1,
                                     line_len,
                                     block.buffer.data.data() + begin_line);
        if (angle_ >= 45 ||
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
            col_idx_real += 1 / (3 * slope_begin);  // experimental
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