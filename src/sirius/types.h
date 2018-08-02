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

#ifndef SIRIUS_TYPES_H_
#define SIRIUS_TYPES_H_

#include <cmath>

#include <array>
#include <string>
#include <vector>

namespace sirius {

using Buffer = std::vector<double>;

/**
 * \brief Data class that represents the size of an image
 */
struct Size {
    Size() = default;

    Size(int row, int col);
    Size(const std::array<int, 2>& size);

    ~Size() = default;
    Size(const Size&) = default;
    Size& operator=(const Size&) = default;
    Size(Size&&) = default;
    Size& operator=(Size&&) = default;

    bool operator<(const Size& rhs) const {
        return (row < rhs.row) || ((row == rhs.row) && (col < rhs.col));
    }

    bool operator==(const Size& rhs) const {
        return row == rhs.row && col == rhs.col;
    }

    Size operator*(int scale) const {
        Size result(*this);
        result.row *= scale;
        result.col *= scale;
        return result;
    }

    Size operator*(double scale) const {
        Size result(*this);
        result.row = std::ceil(result.row * scale);
        result.col = std::ceil(result.col * scale);
        return result;
    }

    Size& operator*=(int scale) {
        *this = *this * scale;
        return *this;
    }

    int CellCount() const { return row * col; }

    int row{0};
    int col{0};
};

/**
 * \brief Data class that represents the 2D coordinates of a point
 */
struct Point {
    Point() = default;

    constexpr Point(int x, int y) noexcept : x(x), y(y) {}

    ~Point() = default;
    Point(const Point&) = default;
    Point& operator=(const Point&) = default;
    Point(Point&&) = default;
    Point& operator=(Point&&) = default;

    int x{0};
    int y{0};
};

/**
 * \brief Data class that represents zoom ratio as
 *        input_resolution/output_resolution
 */
class ZoomRatio {
  public:
    /**
     * \brief Create an instance from a formatted string
     *        (input_resolution:output_resolution)
     * \param ratio_string should be formatted as
     *        "input_resolution:output_resolution"
     * \return zoom ratio
     *
     * \throw sirius::Exception if string format or ratio is invalid
     */
    static ZoomRatio Create(const std::string& ratio_string);

    /**
     * \brief Zoom ratio as input_resolution/output_resolution
     *        Reduce the ratio
     * \param input_resolution numerator of the ratio
     * \param output_resolution denominator of the ratio
     * \return zoom ratio
     *
     * \throw sirius::Exception if ratio is invalid
     * \throw std::invalid_argument if conversion from string to int failed
     */
    static ZoomRatio Create(int input_resolution, int output_resolution = 1);

    /**
     * \brief Instantiate a zoom ratio 1:1
     */
    ZoomRatio() = default;

    ~ZoomRatio() = default;
    ZoomRatio(const ZoomRatio&) = default;
    ZoomRatio(ZoomRatio&&) = default;
    ZoomRatio& operator=(const ZoomRatio&) = default;
    ZoomRatio& operator=(ZoomRatio&&) = default;

    int input_resolution() const { return input_resolution_; }

    int output_resolution() const { return output_resolution_; }

    double ratio() const {
        return input_resolution_ / static_cast<double>(output_resolution_);
    }

    bool IsRealZoom() const;

  private:
    ZoomRatio(int input_resolution, int output_resolution);

    void ReduceRatio();

  private:
    int input_resolution_{1};
    int output_resolution_{1};
};

}  // namespace sirius

#endif  // SIRIUS_TYPES_H_
