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

#include "sirius/image.h"

#include <cstring>

#include <algorithm>
#include <utility>

#include "sirius/utils/log.h"

namespace sirius {

Image::Image(const Size& size) : size(size), data(size.CellCount(), 0) {}

Image::Image(const Size& size, Buffer&& buf)
    : size(size), data(std::move(buf)) {}

Image::Image(Image&& rhs)
    : size(std::exchange(rhs.size, {0, 0})),
      data(std::exchange(rhs.data, {})) {}

Image& Image::operator=(Image&& rhs) {
    size = std::exchange(rhs.size, {0, 0});
    data = std::exchange(rhs.data, {});
    return *this;
}

Image Image::CreatePaddedImage(const Padding& padding) const {
    if (padding.IsEmpty()) {
        return {*this};
    }

    switch (padding.type) {
        case PaddingType::kZeroPadding:
            return CreateZeroPaddedImage(padding);
        case PaddingType::kMirrorPadding:
            return CreateMirrorPaddedImage(padding);
        default:
            LOG("image", warn, "padding type not handled, zero pad image");
            return CreateZeroPaddedImage(padding);
    }
}

Image Image::CreateZeroPaddedImage(const Padding& padding) const {
    LOG("image", trace, "zero pad image {}x{} by ({}, {}, {}, {})", size.row,
        size.col, padding.top, padding.bottom, padding.left, padding.right);
    int row_count = size.row + padding.top + padding.bottom;
    int col_count = size.col + padding.left + padding.right;

    Image result({row_count, col_count});

    std::fill(result.data.begin(), result.data.end(), 0);

    // left and right zero padding
    // beginning of the first real image's line
    int top_offset = col_count * padding.top;

    for (int i_row = 0; i_row < size.row; ++i_row) {
        // copy row
        auto result_row_index = top_offset + (i_row * col_count) + padding.left;

        auto begin_result_it = result.data.begin() + result_row_index;
        auto begin_row_it = data.cbegin() + i_row * size.col;
        auto end_row_it = begin_row_it + size.col;

        std::copy(begin_row_it, end_row_it, begin_result_it);
    }

    return result;
}

Image Image::CreateMirrorPaddedImage(const Padding& padding) const {
    LOG("image", trace, "mirror pad image {}x{} by ({}, {}, {}, {})", size.row,
        size.col, padding.top, padding.bottom, padding.left, padding.right);
    int row_count = size.row + padding.top + padding.bottom;
    int col_count = size.col + padding.left + padding.right;

    Image result({row_count, col_count});

    std::fill(result.data.begin(), result.data.end(), 0);

    // top mirroring
    for (int i = 0; i < padding.top; ++i) {
        int result_row_offset = i * col_count + padding.left;
        int data_row_offset = (padding.top - 1 - i) * size.col;

        std::copy(data.begin() + data_row_offset,
                  data.begin() + data_row_offset + size.col,
                  result.data.begin() + result_row_offset);
    }

    // copy original image
    // beginning of the first real image's line
    int top_offset = col_count * padding.top;
    for (int i_row = 0; i_row < size.row; ++i_row) {
        // copy row
        auto result_row_index = top_offset + (i_row * col_count) + padding.left;

        auto begin_result_it = result.data.begin() + result_row_index;
        auto begin_row_it = data.cbegin() + i_row * size.col;
        auto end_row_it = begin_row_it + size.col;

        std::copy(begin_row_it, end_row_it, begin_result_it);
    }

    // bottom mirroring
    int bottom_offset = row_count * col_count - padding.bottom * col_count;
    int end_data = size.col * size.row;
    for (int row = 0; row < padding.bottom; ++row) {
        int result_row_offset = bottom_offset + padding.left + row * col_count;
        int data_row_offset = end_data - (row + 1) * size.col;

        std::copy(data.begin() + data_row_offset,
                  data.begin() + data_row_offset + size.col,
                  result.data.begin() + result_row_offset);
    }

    // left mirroring
    for (int row = 0; row < row_count; ++row) {
        for (int col = 0; col < padding.left; ++col) {
            result.Set(row, col, result.Get(row, 2 * padding.left - col - 1));
        }
    }

    // right mirroring
    for (int row = 0; row < row_count; ++row) {
        for (int col = 0; col < padding.right; ++col) {
            result.Set(row, size.col + padding.left + col,
                       result.Get(row, size.col + padding.left - col - 1));
        }
    }

    return result;
}

void Image::CreateEvenImage() {
    LOG("image", trace, "Resize image to pair dimensions");
    auto new_size = size;
    bool odd_row = false;
    bool odd_col = false;
    if (size.row % 2 != 0) {
        new_size.row += 1;
        odd_row = true;
    }
    if (size.col % 2 != 0) {
        new_size.col += 1;
        odd_col = true;
    }

    Image output_image(new_size);
    auto src_it = data.begin();
    auto dst_it = output_image.data.begin();
    for (int i = 0; i < size.row; ++i) {
        std::copy(src_it, src_it + size.col, dst_it);
        src_it += size.col;
        dst_it += size.col;
        if (odd_col) {
            *dst_it = *(dst_it - 1);
            dst_it++;
        }
    }

    int x_size = size.col;
    if (odd_col) x_size++;
    if (odd_row) {
        std::copy(dst_it - x_size, dst_it, dst_it);
    }

    data = output_image.data;
    size = output_image.size;
}

}  // namespace sirius
