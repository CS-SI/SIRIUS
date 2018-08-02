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

#ifndef SIRIUS_IMAGE_H_
#define SIRIUS_IMAGE_H_

#include <cassert>

#include "sirius/types.h"

namespace sirius {

enum class PaddingType {
    kZeroPadding = 0,   /**<  zero padding: fill margins with 0*/
    kMirrorPadding = 1, /**<  mirror padding: duplicate rows/cols in mirror*/
};

/**
 * \brief Data class that represents the padding of an image
 */
struct Padding {
    Padding() = default;
    Padding(int top, int bottom, int left, int right,
            PaddingType type = PaddingType::kMirrorPadding);

    ~Padding() = default;
    Padding(const Padding&) = default;
    Padding& operator=(const Padding&) = default;
    Padding(Padding&&) = default;
    Padding& operator=(Padding&&) = default;

    int top{0};
    int bottom{0};
    int left{0};
    int right{0};

    bool IsEmpty() const {
        return (top == 0 && bottom == 0 && left == 0 && right == 0);
    }

    PaddingType type{PaddingType::kMirrorPadding};
};

/**
 * \brief Data class that represents an image (Size + Buffer)
 */
class Image {
  public:
    Image() = default;

    /**
     * \brief Instanciate an image of the given size and pre-allocate its buffer
     *        with 0
     * \param size image size
     */
    explicit Image(const Size& size);

    /**
     * \brief Instanciate an image of the given size with data
     * \param size image size
     * \param buffer image buffer
     */
    Image(const Size& size, Buffer&& buffer);

    ~Image() = default;

    Image(const Image&) = default;
    Image& operator=(const Image&) = default;
    Image(Image&&);
    Image& operator=(Image&&);

    /**
     * \brief Get the cell count of the image (row x col)
     * \return cell count
     */
    int CellCount() const { return size.CellCount(); }

    /**
     * \brief Get the value at cell (row, col)
     *        Row and col starts at 0
     * \return value
     */
    Buffer::value_type Get(int row, int col) const {
        assert(row < size.row && col < size.col);

        return data[row * size.col + col];
    }

    /**
     * \brief Set the value at cell (row, col)
     *        Row and col starts at 0
     */
    void Set(int row, int col, Buffer::value_type val) {
        assert(row < size.row && col < size.col);

        data[row * size.col + col] = val;
    }

    /**
     * \brief Check that the image is loaded
     *        Row, col and data are set
     * \return bool
     */
    bool IsLoaded() const {
        return size.row != 0 && size.col != 0 &&
               data.size() >= static_cast<std::size_t>(CellCount());
    }

    /**
     * \brief Create padded image from the current image
     *
     * Padding strategy is selected according to padding.type
     *
     * \param padding padding to apply
     * \return generated image
     */
    Image CreatePaddedImage(const Padding& padding) const;

    /**
     * \brief Create a zero padded image from the current image
     * \param zero_padding padding to apply
     * \return generated image
     */
    Image CreateZeroPaddedImage(const Padding& zero_padding) const;

    /**
     * \brief Create a padded image using mirroring on borders
     * \param padding_size size of the margins
     * \return generated image
     */
    Image CreateMirrorPaddedImage(const Padding& mirror_padding) const;

    /**
     * \brief add row and col according to odd dim of calling image
     */
    void CreateEvenImage();

  public:
    Size size{0, 0};
    Buffer data;
};

}  // namespace sirius

#endif  // SIRIUS_IMAGE_H_
