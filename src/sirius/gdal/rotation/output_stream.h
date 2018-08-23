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
     * \param block image contained in its minimal non rotated rectangular
     * \param angle rotation angle
     * \param tr top right corner
     * \param tl top left corner
     * \param br bottom right corner
     * \param bl bottom left corner
     */
    void CopyConvexHull(const sirius::gdal::StreamBlock& block, const int angle,
                        const Point& tr, const Point& tl, const Point& br,
                        const Point& bl);

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
};

}  // namespace rotation
}  // namespace gdal
}  // namespace sirius

#endif  // SIRIUS_GDAL_ROTATION_OUTPUT_STREAM_H_