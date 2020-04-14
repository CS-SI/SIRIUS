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

#ifndef SIRIUS_GDAL_ROTATION_OUTPUT_STREAM_H_
#define SIRIUS_GDAL_ROTATION_OUTPUT_STREAM_H_

#include <string>

#include "sirius/types.h"

#include "sirius/gdal/i_output_stream.h"
#include "sirius/gdal/output_stream.h"

#include "sirius/rotation/parameters.h"

namespace sirius {
namespace gdal {
namespace rotation {

class OutputStream : public IOutputStream {
  public:
    OutputStream(const std::string& input_path, const std::string& output_path,
                 const sirius::rotation::Parameters& rotation_parameters);

    ~OutputStream() = default;
    OutputStream(const OutputStream&) = delete;
    OutputStream& operator=(const OutputStream&) = delete;
    OutputStream(OutputStream&&) = default;
    OutputStream& operator=(OutputStream&&) = default;

    /**
     * \brief Write a translated block in the output file
     * \param block block to write
     * \param ec error code if operation failed
     */
    void Write(StreamBlock&& block, std::error_code& ec) override;

  private:
    /**
     * \brief copy data inside the convex hull
     * \param block image contained in its minimal non rotated rectangle
     * \param begin_marged_block marged blocks indexes where we may start
     * copying (if no top & left margins)
     * \param tl rotated block with margins' top left corner coordinates
     */
    void CopyConvexHull(const sirius::gdal::StreamBlock& block,
                        Point& begin_marged_block, const Point& tl);

    /**
     * \brief copy part of the hull from one distinctive point to another
     * \param block image contained in its minimal non rotated rectangle
     * \param begin_line index at which we start reading block
     * \param end_line, index at which we stop reading block
     * \param begin_line_real used to compute the exact beginning index (without
     * rounding)
     * \param end_line_real used to compute the exact ending index (without
     * rounding)
     * \param row_idx y coordinate at which we start reading block
     * \param col_idx x coordinate at which we start reading block
     * \param col_idx_real used to compute the exact x coordinate (without
     * rounding)
     * \param first_y y coordinate used to start iterating
     * \param second_y y coordinate used to stop iterating
     * \param slope_begin slope of the line joining the two points on the
     * left of the rectangle
     * \param slope_end slope of the line joining the two points on the
     * right of the rectangle
     */
    void CopyHullPart(const sirius::gdal::StreamBlock& block, int& begin_line,
                      int& end_line, double& begin_line_real,
                      double& end_line_real, int& row_idx, int& col_idx,
                      double& col_idx_real, int first_y, int second_y,
                      double slope_begin, double slope_end);

  private:
    gdal::OutputStream output_stream_;
    GDALDataset* output_dataset_;
    int angle_;
    double slope_tr_br_ = 0;
    double slope_tr_tl_ = 0;
    double slope_br_bl_ = 0;
    double slope_tl_bl_ = 0;
    double slope_tl_tr_ = 0;
    double slope_bl_br_ = 0;
    Size block_margin_size_ = {50, 50};
    Size tl_tr_vector_;
    Size tr_br_vector_;
    Size tl_bl_vector_;
    Size bl_br_vector_;
    Size br_bl_vector_;
    Size tr_tl_vector_;
};

}  // namespace rotation
}  // namespace gdal
}  // namespace sirius

#endif  // SIRIUS_GDAL_ROTATION_OUTPUT_STREAM_H_