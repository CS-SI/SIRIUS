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

#include <catch/catch.hpp>

#include "sirius/image.h"

#include "sirius/gdal/exception.h"
#include "sirius/gdal/wrapper.h"

#include "sirius/utils/log.h"

#include "utils.h"

namespace sirius {
namespace tests {

void CheckZeroPaddingImage(const Image& image, const Image& padded_image,
                           const Padding& padding);

void CheckMirrorPaddingImage(const Image& image, const Image& padded_image,
                             const Padding& padding);

}  // namespace tests
}  // namespace sirius

TEST_CASE("Image - Basics move", "[image]") {
    static constexpr int kRow = 10;
    static constexpr int kCol = 20;
    sirius::Image image({kRow, kCol});

    sirius::Image moved_image(std::move(image));
    REQUIRE(image.size.row == 0);
    REQUIRE(image.size.col == 0);
    REQUIRE(image.data.size() == 0);

    REQUIRE(moved_image.size.row == kRow);
    REQUIRE(moved_image.size.col == kCol);
    REQUIRE(moved_image.data.size() == moved_image.size.CellCount());

    sirius::Image moved_image_assign;
    moved_image_assign = std::move(moved_image);
    REQUIRE(moved_image.size.row == 0);
    REQUIRE(moved_image.size.col == 0);
    REQUIRE(moved_image.data.size() == 0);

    REQUIRE(moved_image_assign.size.row == kRow);
    REQUIRE(moved_image_assign.size.col == kCol);
    REQUIRE(moved_image_assign.data.size() ==
            moved_image_assign.size.CellCount());
}

TEST_CASE("Image - Zero pad image", "[image]") {
    LOG_SET_LEVEL(trace);

    auto input = sirius::tests::CreateDummyImage({5, 5});

    SECTION("No padding") {
        sirius::Padding padding(0, 0, 0, 0, sirius::PaddingType::kZeroPadding);
        auto output = input.CreateZeroPaddedImage(padding);

        REQUIRE(input.size == output.size);

        // identical image
        sirius::tests::CheckZeroPaddingImage(input, output, padding);
    }

    SECTION("Padding top") {
        sirius::Padding padding(5, 0, 0, 0, sirius::PaddingType::kZeroPadding);
        auto output = input.CreateZeroPaddedImage(padding);

        sirius::tests::CheckZeroPaddingImage(input, output, padding);
    }

    SECTION("Padding bottom") {
        sirius::Padding padding(0, 5, 0, 0, sirius::PaddingType::kZeroPadding);
        auto output = input.CreateZeroPaddedImage(padding);
        sirius::tests::CheckZeroPaddingImage(input, output, padding);
    }

    SECTION("Padding left") {
        sirius::Padding padding(0, 0, 5, 0, sirius::PaddingType::kZeroPadding);
        auto output = input.CreateZeroPaddedImage(padding);

        sirius::tests::CheckZeroPaddingImage(input, output, padding);
    }

    SECTION("Padding right") {
        sirius::Padding padding(0, 0, 0, 5, sirius::PaddingType::kZeroPadding);
        auto output = input.CreateZeroPaddedImage(padding);

        sirius::tests::CheckZeroPaddingImage(input, output, padding);
    }

    SECTION("Padding mix") {
        sirius::Padding padding(3, 2, 1, 5, sirius::PaddingType::kZeroPadding);
        auto output = input.CreateZeroPaddedImage(padding);

        sirius::tests::CheckZeroPaddingImage(input, output, padding);
    }

    SECTION("Padding mix 2") {
        sirius::Padding padding(10, 0, 2, 6, sirius::PaddingType::kZeroPadding);
        auto output = input.CreateZeroPaddedImage(padding);

        sirius::tests::CheckZeroPaddingImage(input, output, padding);
    }

    SECTION("Padding mix 3") {
        sirius::Padding padding(0, 10, 3, 7, sirius::PaddingType::kZeroPadding);
        auto output = input.CreateZeroPaddedImage(padding);

        sirius::tests::CheckZeroPaddingImage(input, output, padding);
    }
}

TEST_CASE("Image - Mirror pad image", "[image]") {
    LOG_SET_LEVEL(trace);

    auto input = sirius::tests::CreateDummyImage({5, 5});

    SECTION("No padding") {
        sirius::Padding padding(0, 0, 0, 0,
                                sirius::PaddingType::kMirrorPadding);
        auto output = input.CreateMirrorPaddedImage(padding);

        REQUIRE(input.size == output.size);
        sirius::tests::CheckMirrorPaddingImage(input, output, padding);
    }

    SECTION("Padding top") {
        sirius::Padding padding(2, 0, 0, 0,
                                sirius::PaddingType::kMirrorPadding);
        auto output = input.CreateMirrorPaddedImage(padding);

        sirius::tests::CheckMirrorPaddingImage(input, output, padding);
    }

    SECTION("Padding bottom") {
        sirius::Padding padding(0, 2, 0, 0,
                                sirius::PaddingType::kMirrorPadding);
        auto output = input.CreateMirrorPaddedImage(padding);

        sirius::tests::CheckMirrorPaddingImage(input, output, padding);
    }

    SECTION("Padding left") {
        sirius::Padding padding(0, 0, 2, 0,
                                sirius::PaddingType::kMirrorPadding);
        auto output = input.CreateMirrorPaddedImage(padding);

        sirius::tests::CheckMirrorPaddingImage(input, output, padding);
    }

    SECTION("Padding right") {
        sirius::Padding padding(0, 0, 0, 2,
                                sirius::PaddingType::kMirrorPadding);
        auto output = input.CreateMirrorPaddedImage(padding);

        sirius::tests::CheckMirrorPaddingImage(input, output, padding);
    }

    SECTION("Padding mix") {
        sirius::Padding padding(1, 3, 1, 3,
                                sirius::PaddingType::kMirrorPadding);
        auto output = input.CreateMirrorPaddedImage(padding);

        sirius::tests::CheckMirrorPaddingImage(input, output, padding);
    }

    SECTION("Padding mix 2") {
        sirius::Padding padding(3, 1, 3, 1,
                                sirius::PaddingType::kMirrorPadding);
        auto output = input.CreateMirrorPaddedImage(padding);

        sirius::tests::CheckMirrorPaddingImage(input, output, padding);
    }

    SECTION("Padding mix 3") {
        sirius::Padding padding(3, 2, 0, 3,
                                sirius::PaddingType::kMirrorPadding);
        auto output = input.CreateMirrorPaddedImage(padding);

        sirius::tests::CheckMirrorPaddingImage(input, output, padding);
    }

    SECTION("Padding mix 3") {
        sirius::Padding padding(3, 3, 3, 3,
                                sirius::PaddingType::kMirrorPadding);
        auto output = input.CreateMirrorPaddedImage(padding);

        sirius::tests::CheckMirrorPaddingImage(input, output, padding);
    }
}

TEST_CASE("Image - load empty path", "[sirius]") {
    LOG_SET_LEVEL(trace);

    sirius::Image image;
    REQUIRE_NOTHROW(image = sirius::gdal::LoadImage(""));

    REQUIRE(image.data.empty());
    REQUIRE(image.size.row == 0);
    REQUIRE(image.size.col == 0);
}

TEST_CASE("GDAL - load unknown image", "[sirius]") {
    LOG_SET_LEVEL(trace);

    sirius::Image image;
    REQUIRE_THROWS_AS(
          image = sirius::gdal::LoadImage("/this/is/not/a/file.ext"),
          sirius::gdal::Exception);

    REQUIRE(image.data.empty());
    REQUIRE(image.size.row == 0);
    REQUIRE(image.size.col == 0);
}

TEST_CASE("Image - load invalid image", "[sirius]") {
    LOG_SET_LEVEL(trace);

    sirius::Image image;
    REQUIRE_THROWS_AS(
          image = sirius::gdal::LoadImage("/this/is/not/a/file.ext"),
          sirius::gdal::Exception);

    REQUIRE(image.data.empty());
    REQUIRE(image.size.row == 0);
    REQUIRE(image.size.col == 0);
}

TEST_CASE("Image - load well-formed image", "[sirius]") {
    LOG_SET_LEVEL(trace);

    sirius::Image image;

    REQUIRE_NOTHROW(
          image = sirius::gdal::LoadImage("./filters/dirac_filter.tiff"));
    REQUIRE(!image.data.empty());
    REQUIRE(image.size.row > 0);
    REQUIRE(image.size.col > 0);
    REQUIRE(image.IsLoaded());

    REQUIRE_NOTHROW(
          image = sirius::gdal::LoadImage("./filters/sinc_zoom2_filter.tif"));
    REQUIRE(!image.data.empty());
    REQUIRE(image.size.row > 0);
    REQUIRE(image.size.col > 0);
    REQUIRE(image.IsLoaded());
}

namespace sirius {
namespace tests {

void CheckZeroPaddingImage(const Image& image, const Image& padded_image,
                           const Padding& padding) {
    REQUIRE(padded_image.size.row ==
            (image.size.row + padding.top + padding.bottom));
    REQUIRE(padded_image.size.col ==
            (image.size.col + padding.left + padding.right));
    REQUIRE(padded_image.data.size() == padded_image.size.CellCount());

    // check top padding
    for (int pad_row = 0; pad_row < padding.top; ++pad_row) {
        for (int col = 0; col < padded_image.size.col; ++col) {
            REQUIRE(padded_image.Get(pad_row, col) == 0.);
        }
    }

    // check bottom padding
    for (int pad_row = 0; pad_row < padding.bottom; ++pad_row) {
        for (int col = 0; col < padded_image.size.col; ++col) {
            REQUIRE(padded_image.Get(padded_image.size.row - pad_row - 1,
                                     col) == 0.);
        }
    }

    // check left padding
    for (int pad_col = 0; pad_col < padding.left; ++pad_col) {
        for (int row = 0; row < padded_image.size.row; ++row) {
            REQUIRE(padded_image.Get(row, pad_col) == 0.);
        }
    }

    // check right padding
    for (int pad_col = 0; pad_col < padding.right; ++pad_col) {
        for (int row = 0; row < padded_image.size.row; ++row) {
            REQUIRE(padded_image.Get(
                          row, padded_image.size.col - pad_col - 1) == 0.);
        }
    }

    // check image
    for (int row = 0; row < image.size.row; ++row) {
        for (int col = 0; col < image.size.col; ++col) {
            REQUIRE(padded_image.Get(row + padding.top, col + padding.left) ==
                    image.Get(row, col));
        }
    }
}

void CheckMirrorPaddingImage(const Image& image, const Image& padded_image,
                             const Padding& padding) {
    REQUIRE(padded_image.size.row ==
            (image.size.row + padding.top + padding.bottom));
    REQUIRE(padded_image.size.col ==
            (image.size.col + padding.left + padding.right));
    REQUIRE(padded_image.data.size() == padded_image.size.CellCount());

    // check top padding
    for (int row = 0; row < padding.top; ++row) {
        for (int col = padding.left;
             col < padded_image.size.col - padding.right; ++col) {
            REQUIRE(padded_image.Get(row, col) ==
                    image.Get(padding.top - row - 1, col - padding.left));
        }
    }

    // check bottom padding
    for (int row = 0; row < padding.bottom; ++row) {
        for (int col = padding.left;
             col < padded_image.size.col - padding.right; ++col) {
            REQUIRE(padded_image.Get(padded_image.size.row - row - 1, col) ==
                    image.Get(image.size.row - padding.bottom + row,
                              col - padding.left));
        }
    }

    // check left padding
    for (int col = 0; col < padding.left; ++col) {
        for (int row = 0; row < padded_image.size.row; ++row) {
            REQUIRE(padded_image.Get(row, col) ==
                    padded_image.Get(row, 2 * padding.left - col - 1));
        }
    }

    // check right padding
    for (int row = 0; row < padded_image.size.row; ++row) {
        for (int col = 0; col < padding.right; ++col) {
            REQUIRE(padded_image.Get(row, padded_image.size.col - col - 1) ==
                    padded_image.Get(row, padded_image.size.col -
                                                2 * padding.right + col));
        }
    }

    // check image
    for (int row = 0; row < image.size.row; ++row) {
        for (int col = 0; col < image.size.col; ++col) {
            REQUIRE(padded_image.Get(row + padding.top, col + padding.left) ==
                    image.Get(row, col));
        }
    }
}

}  // namespace tests
}  // namespace sirius
