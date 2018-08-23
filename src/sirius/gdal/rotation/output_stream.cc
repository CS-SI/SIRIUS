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

    ///////
    GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
    GDALDataset* dataset = driver->Create(
          "/home/mbelloc/tmp/rotated_block.tif", block.buffer.size.col,
          block.buffer.size.row, 1, GDT_Float32, NULL);
    dataset->GetRasterBand(1)->RasterIO(
          GF_Write, 0, 0, block.buffer.size.col, block.buffer.size.row,
          block.buffer.data.data(), block.buffer.size.col,
          block.buffer.size.row, GDT_Float64, 0, 0, NULL);
    GDALClose(dataset);
    //////

    Point tr, tl, br, bl;
    sirius::rotation::RecoverCorners(block.original_size, angle_,
                                     block.buffer.size, tr, tl, br, bl);
    CopyConvexHull(block, angle_, tr, tl, br, bl);
    ec = make_error_code(CPLE_None);
}

void OutputStream::CopyConvexHull(const sirius::gdal::StreamBlock& block,
                                  const int angle, const Point& tr,
                                  const Point& tl, const Point& br,
                                  const Point& bl) {
    //////
    // Image test_block_copy(block.buffer.size);
    //////
    Size output_size(output_dataset_->GetRasterYSize(),
                     output_dataset_->GetRasterXSize());
    LOG("OutputStream", debug, "hull size = {}x{}", block.buffer.size.row,
        block.buffer.size.col);

    int row_idx = block.row_idx;
    int col_idx = block.col_idx;
    double col_idx_real = col_idx;

    if (angle_ > 0) {
        row_idx += tr.y;
        col_idx += tr.x;
        col_idx_real += tr.x;
    } else {
        row_idx += tl.y;
        col_idx += tl.x;
        col_idx_real += tl.x;
    }

    if (angle >= 0) {
        Point first_point;
        Point second_point;
        if (angle > 45) {
            if (block.original_size.row <= block.original_size.col) {
                first_point = br;
                second_point = tl;
            } else {
                first_point = tl;
                second_point = br;
            }
        } else {
            if (block.original_size.row <= block.original_size.col) {
                first_point = tl;
                second_point = br;
            } else {
                first_point = br;
                second_point = tl;
            }
        }

        LOG("OutputStream", debug,
            "first point = {}, {}, second point = {}, {}", first_point.x,
            first_point.y, second_point.x, second_point.y);

        int begin_line = tr.x - 1;
        int end_line = tr.x;
        double begin_line_real = begin_line;
        double end_line_real = end_line;
        double slope_begin = slope_tr_tl_;
        double slope_end = slope_tr_br_;
        // copy from the point with coordinate y=0 to the first point of the
        // hull
        LOG("OutputStream", debug, "copy first part");
        for (int i = 0; i < first_point.y; ++i) {
            LOG("OutputStream", debug, "begin_line = {}, end_line = {}",
                begin_line, end_line);
            LOG("OutputStream", debug, "row_idx = {}, col_idx = {}", row_idx,
                col_idx);
            //////
            /*std::copy(block.buffer.data.data() + begin_line,
                      block.buffer.data.data() + end_line,
                      test_block_copy.data.data() + begin_line);*/
            //////
            CPLErr err = output_dataset_->GetRasterBand(1)->RasterIO(
                  GF_Write, col_idx, row_idx, end_line - begin_line, 1,
                  const_cast<double*>(block.buffer.data.data()) + begin_line,
                  end_line - begin_line, 1, GDT_Float64, 0, 0, NULL);
            if (err) {
                LOG("output_stream", error,
                    "GDAL error: {} - could not write to the given dataset",
                    err);
                return;
            }

            begin_line_real += block.buffer.size.col + 1 / slope_begin;
            end_line_real += block.buffer.size.col + 1 / slope_end;
            begin_line = std::round(begin_line_real);
            end_line = std::round(end_line_real);
            row_idx++;
            col_idx_real += 1 / slope_begin;
            col_idx = std::round(col_idx_real);
        }

        // updates slopes that allows us to know when to start and stop reading
        // a line
        if (angle > 45) {
            if (block.original_size.row <= block.original_size.col) {
                slope_end = slope_br_bl_;
            } else {
                slope_begin = slope_tl_bl_;
            }
        } else {
            if (block.original_size.row <= block.original_size.col) {
                slope_begin = slope_tl_bl_;
            } else {
                slope_end = slope_br_bl_;
            }
        }

        // copy from the first point with coordinate y != 0 to the second point
        // of the hull
        LOG("OutputStream", debug, "copy second part");
        for (int i = first_point.y; i < second_point.y; ++i) {
            LOG("OutputStream", debug, "begin_line = {}, end_line = {}",
                begin_line, end_line);
            LOG("OutputStream", debug, "row_idx = {}, col_idx = {}", row_idx,
                col_idx);
            //////
            /*std::copy(block.buffer.data.data() + begin_line,
                      block.buffer.data.data() + end_line,
                      test_block_copy.data.data() + begin_line);*/
            //////

            CPLErr err = output_dataset_->GetRasterBand(1)->RasterIO(
                  GF_Write, col_idx, row_idx, end_line - begin_line, 1,
                  const_cast<double*>(block.buffer.data.data()) + begin_line,
                  end_line - begin_line, 1, GDT_Float64, 0, 0, NULL);
            if (err) {
                LOG("output_stream", error,
                    "GDAL error: {} - could not write to the given dataset",
                    err);
                return;
            }
            begin_line_real += block.buffer.size.col + 1 / slope_begin;
            end_line_real += block.buffer.size.col + 1 / slope_end;
            begin_line = std::round(begin_line_real);
            end_line = std::round(end_line_real);
            row_idx++;
            col_idx_real += 1 / slope_begin;
            col_idx = std::round(col_idx_real);
        }

        // update slopes
        if (angle > 45) {
            if (block.original_size.row <= block.original_size.col) {
                slope_begin = slope_tl_bl_;
            } else {
                slope_end = slope_br_bl_;
            }
        } else {
            if (block.original_size.row <= block.original_size.col) {
                slope_end = slope_br_bl_;
            } else {
                slope_begin = slope_tl_bl_;
            }
        }

        // copy from the second point we encountered until we reach image's
        // height
        LOG("OutputStream", debug, "copy third part");
        for (int i = second_point.y; i < block.buffer.size.row; ++i) {
            LOG("OutputStream", debug, "begin_line = {}, end_line = {}",
                begin_line, end_line);
            LOG("OutputStream", debug, "row_idx = {}, col_idx = {}", row_idx,
                col_idx);
            //////
            /*std::copy(block.buffer.data.data() + begin_line,
                      block.buffer.data.data() + end_line,
                      test_block_copy.data.data() + begin_line);*/
            //////
            CPLErr err = output_dataset_->GetRasterBand(1)->RasterIO(
                  GF_Write, col_idx, row_idx, end_line - begin_line, 1,
                  const_cast<double*>(block.buffer.data.data()) + begin_line,
                  end_line - begin_line, 1, GDT_Float64, 0, 0, NULL);
            if (err) {
                LOG("output_stream", error,
                    "GDAL error: {} - could not write to the given dataset",
                    err);
                return;
            }
            begin_line_real += block.buffer.size.col + 1 / slope_begin;
            end_line_real += block.buffer.size.col + 1 / slope_end;
            begin_line = std::round(begin_line_real);
            end_line = std::round(end_line_real);
            row_idx++;
            col_idx_real += 1 / slope_begin;
            col_idx = std::round(col_idx_real);
        }
    } else {
        Point first_point;
        Point second_point;
        if (angle < -45) {
            if (block.original_size.row <= block.original_size.col) {
                first_point = bl;
                second_point = tr;
            } else {
                first_point = tr;
                second_point = bl;
            }
        } else {
            if (block.original_size.row <= block.original_size.col) {
                first_point = tr;
                second_point = bl;
            } else {
                first_point = bl;
                second_point = tr;
            }
        }

        int begin_line = tl.x + 1;
        int end_line = tl.x + 2;
        double begin_line_real = begin_line;
        double end_line_real = end_line;
        double slope_begin = slope_tl_bl_;
        double slope_end = slope_tl_tr_;
        // copy from the point with coordinate y=0 to the first point of the
        // hull
        for (int i = 0; i < first_point.y; ++i) {
            /*LOG("OutputStream", debug, "begin_line = {}, end_line = {}",
                begin_line, end_line);
            LOG("OutputStream", debug, "row_idx = {}, col_idx = {}", row_idx,
                col_idx);*/
            //////
            /*std::copy(block.buffer.data.data() + begin_line,
                      block.buffer.data.data() + end_line,
                      test_block_copy.data.data() + begin_line);*/
            //////
            CPLErr err = output_dataset_->GetRasterBand(1)->RasterIO(
                  GF_Write, col_idx, row_idx, end_line - begin_line, 1,
                  const_cast<double*>(block.buffer.data.data()) + begin_line,
                  end_line - begin_line, 1, GDT_Float64, 0, 0, NULL);
            if (err) {
                LOG("output_stream", error,
                    "GDAL error: {} - could not write to the given dataset",
                    err);
                return;
            }
            begin_line_real += block.buffer.size.col + 1 / slope_begin;
            end_line_real += block.buffer.size.col + 1 / slope_end;
            begin_line = std::round(begin_line_real);
            end_line = std::round(end_line_real);
            row_idx++;
            col_idx_real += 1 / slope_begin;
            col_idx = std::round(col_idx_real);
        }

        // updates slopes that allows us to know when to start and stop reading
        // a line
        if (angle < -45) {
            if (block.original_size.row <= block.original_size.col) {
                slope_begin = slope_bl_br_;
            } else {
                slope_end = slope_tr_br_;
            }
        } else {
            if (block.original_size.row <= block.original_size.col) {
                slope_end = slope_tr_br_;
            } else {
                slope_begin = slope_bl_br_;
            }
        }

        // copy from the first point with coordinate y != 0 to the second point
        // of the hull
        for (int i = first_point.y; i < second_point.y; ++i) {
            /*LOG("OutputStream", debug, "begin_line = {}, end_line = {}",
                begin_line, end_line);
            LOG("OutputStream", debug, "row_idx = {}, col_idx = {}", row_idx,
                col_idx);*/
            //////
            /*std::copy(block.buffer.data.data() + begin_line,
                      block.buffer.data.data() + end_line,
                      test_block_copy.data.data() + begin_line);*/
            //////
            CPLErr err = output_dataset_->GetRasterBand(1)->RasterIO(
                  GF_Write, col_idx, row_idx, end_line - begin_line, 1,
                  const_cast<double*>(block.buffer.data.data()) + begin_line,
                  end_line - begin_line, 1, GDT_Float64, 0, 0, NULL);
            if (err) {
                LOG("output_stream", error,
                    "GDAL error: {} - could not write to the given dataset",
                    err);
                return;
            }
            begin_line_real += block.buffer.size.col + 1 / slope_begin;
            end_line_real += block.buffer.size.col + 1 / slope_end;
            begin_line = std::round(begin_line_real);
            end_line = std::round(end_line_real);
            row_idx++;
            col_idx_real += 1 / slope_begin;
            col_idx = std::round(col_idx_real);
        }

        // update slopes
        if (angle < -45) {
            if (block.original_size.row <= block.original_size.col) {
                slope_end = slope_tr_br_;
            } else {
                slope_begin = slope_bl_br_;
            }
        } else {
            if (block.original_size.row <= block.original_size.col) {
                slope_begin = slope_bl_br_;
            } else {
                slope_end = slope_tr_br_;
            }
        }

        // copy from the second point we encountered until we reach image's
        // height
        for (int i = second_point.y; i < block.buffer.size.row; ++i) {
            /*LOG("OutputStream", debug, "begin_line = {}, end_line = {}",
                begin_line, end_line);
            LOG("OutputStream", debug, "row_idx = {}, col_idx = {}", row_idx,
                col_idx);*/
            //////
            /*std::copy(block.buffer.data.data() + begin_line,
                      block.buffer.data.data() + end_line,
                      test_block_copy.data.data() + begin_line);*/
            //////
            CPLErr err = output_dataset_->GetRasterBand(1)->RasterIO(
                  GF_Write, col_idx, row_idx, end_line - begin_line, 1,
                  const_cast<double*>(block.buffer.data.data()) + begin_line,
                  end_line - begin_line, 1, GDT_Float64, 0, 0, NULL);
            if (err) {
                LOG("output_stream", error,
                    "GDAL error: {} - could not write to the given dataset",
                    err);
                return;
            }
            begin_line_real += block.buffer.size.col + 1 / slope_begin;
            end_line_real += block.buffer.size.col + 1 / slope_end;
            begin_line = std::round(begin_line_real);
            end_line = std::round(end_line_real);
            row_idx++;
            col_idx_real += 1 / slope_begin;
            col_idx = std::round(col_idx_real);
        }
    }

    ///////
    /*GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
    GDALDataset* dataset = driver->Create(
          "/home/mbelloc/tmp/copied_block.tif", test_block_copy.size.col,
          test_block_copy.size.row, 1, GDT_Float32, NULL);
    dataset->GetRasterBand(1)->RasterIO(
          GF_Write, 0, 0, test_block_copy.size.col, test_block_copy.size.row,
          test_block_copy.data.data(), test_block_copy.size.col,
          test_block_copy.size.row, GDT_Float64, 0, 0, NULL);
    GDALClose(dataset);
    exit(-1);*/
    ///////
}

}  // namespace rotation
}  // namespace gdal
}  // namespace sirius