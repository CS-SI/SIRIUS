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

#ifndef SIRIUS_GDAL_TRANSLATION_OUTPUT_STREAM_H_
#define SIRIUS_GDAL_TRANSLATION_OUTPUT_STREAM_H_

#include <string>

#include "sirius/types.h"

#include "sirius/gdal/i_output_stream.h"
#include "sirius/gdal/output_stream.h"

#include "sirius/translation/parameters.h"

namespace sirius {
namespace gdal {
namespace translation {

class OutputStream : public IOutputStream {
  public:
    OutputStream(const std::string& input_path, const std::string& output_path,
                 const sirius::translation::Parameters& translation_parameters);

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
    gdal::OutputStream output_stream_;
    ZoomRatio zoom_ratio_;
};

}  // namespace translation
}  // namespace gdal
}  // namespace sirius

#endif  // SIRIUS_GDAL_TRANSLATION_OUTPUT_STREAM_H_