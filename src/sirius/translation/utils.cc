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

#include "sirius/translation/utils.h"

#include "sirius/utils/log.h"

namespace sirius {
namespace translation {

Image RemoveBorders(const Image& image, int row_shift, int col_shift) {
    LOG("translation", trace, "removed size {}x{}", row_shift, col_shift);
    if (col_shift == 0 && row_shift == 0) {
        return image;
    }

    Image output_image(
          {image.size.row - abs(row_shift), image.size.col - abs(col_shift)});

    int begin_row, begin_col, end_row, end_col;
    if (row_shift > 0) {
        begin_row = row_shift;
        end_row = image.size.row;
    } else {
        begin_row = 0;
        end_row = image.size.row + row_shift;
    }

    if (col_shift > 0) {
        begin_col = col_shift;
        end_col = image.size.col;
    } else {
        begin_col = 0;
        end_col = image.size.col + col_shift;
    }

    int nb_elem = end_col - begin_col;
    auto src_it = image.data.begin() + (begin_row * image.size.col + begin_col);
    auto output_it = output_image.data.begin();
    for (int i = begin_row; i < end_row; ++i) {
        std::copy(src_it, src_it + nb_elem, output_it);
        src_it += image.size.col;
        output_it += nb_elem;
    }

    return output_image;
}

}  // namespace translation
}  // namespace sirius