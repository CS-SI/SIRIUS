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

#include "sirius/image_decomposition/periodic_smooth_policy.h"
#include "sirius/image_decomposition/regular_policy.h"

#include "sirius/frequency_rotator_factory.h"

#include "sirius/gdal/wrapper.h"

#include "sirius/utils/log.h"

#include "utils.h"

TEST_CASE("frequency rotater - periodic smooth", "[sirius]") {
    LOG_SET_LEVEL(trace);

    auto frequency_rot =
          sirius::FrequencyRotatorFactory::Create(sirius::image_decomposition::Policies::kPeriodicSmooth);

    // test input
    auto point_image = sirius::gdal::Load(sirius::tests::kPointImagePath); // Image Size 12x8

    sirius::Point i_theBlankPoint(4,2); // start [1, ...]
    int index_i_theBlankPoint = (i_theBlankPoint.y - 1)*point_image.size.col + i_theBlankPoint.x - 1;
    auto point_black_image = sirius::gdal::Load(sirius::tests::kPointBlackImagePath); // Image Size 12x8
    sirius::Point i_theBlackPoint(4,2); // start [1, ...]
    int index_i_theBlackPoint = (i_theBlackPoint.y - 1)*point_black_image.size.col + i_theBlackPoint.x - 1;

    auto lena_image = sirius::gdal::Load(sirius::tests::kLenaImagePath);

    // output
    sirius::Image output;

    SECTION("point - positive rotation +90 deg") {
        sirius::rotation::Parameters params{90};
        REQUIRE_NOTHROW(output = frequency_rot->Compute(point_image, {}, params));
        LOG("tests", debug, "input size: {}, {}", point_image.size.row,
            lena_image.size.col);
        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size.row == point_image.size.col);
        REQUIRE(output.size.col == point_image.size.row);
        sirius::Point o_A(i_theBlankPoint.y,point_image.size.col - i_theBlankPoint.x + 1);
        int index_o_A = (o_A.y - 1)*output.size.col + o_A.x - 1;
        LOG("tests", debug, " i_theBlankPoint {},{} -> index {} -> value {}", i_theBlankPoint.x, i_theBlankPoint.y, index_i_theBlankPoint, point_image.data[index_i_theBlankPoint]);
        LOG("tests", debug, " output A {},{} -> index {}", o_A.x, o_A.y, index_o_A);
        sirius::gdal::Save(output, sirius::tests::kOutputDirectoryPath + "point_rot+90.tif");
        REQUIRE(std::round(point_image.data[index_i_theBlankPoint]) ==
                              std::round(output.data[index_o_A]));

    }
    SECTION("point - negative rotation -90 deg") {
        sirius::rotation::Parameters params{-90};
        REQUIRE_NOTHROW(output = frequency_rot->Compute(point_image, {}, params));
        LOG("tests", debug, "input size: {}, {}", point_image.size.row,
            lena_image.size.col);
        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size.row == point_image.size.col);
        REQUIRE(output.size.col == point_image.size.row);
        sirius::Point o_A(point_image.size.row - i_theBlankPoint.y + 1, i_theBlankPoint.x);
        int index_o_A = (o_A.y - 1)*output.size.col + o_A.x - 1;
        LOG("tests", debug, " i_theBlankPoint {},{} -> index {} -> value {}", i_theBlankPoint.x, i_theBlankPoint.y, index_i_theBlankPoint, point_image.data[index_i_theBlankPoint]);
        LOG("tests", debug, " output A {},{} -> index {}", o_A.x, o_A.y, index_o_A);
        sirius::gdal::Save(output, sirius::tests::kOutputDirectoryPath + "point_rot-90.tif");
        REQUIRE(std::round(point_image.data[index_i_theBlankPoint]) ==
                              std::round(output.data[index_o_A]));

    }

    SECTION("point black - positive rotation +45 deg") {
        sirius::rotation::Parameters params{45};
        REQUIRE_NOTHROW(output = frequency_rot->Compute(point_black_image, {}, params));
        LOG("tests", debug, "input size: {}, {}", point_black_image.size.row,
            point_black_image.size.col);
        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size.row == 15);
        REQUIRE(output.size.col == 15);
        sirius::Point o_A(4, 8);
        int index_o_A = (o_A.y - 1)*output.size.col + o_A.x - 1;
        LOG("tests", debug, " i_theBlackPoint {},{} -> index {} -> value {}", i_theBlackPoint.x, i_theBlackPoint.y, index_i_theBlackPoint, point_black_image.data[index_i_theBlankPoint]);
        LOG("tests", debug, " output A {},{} -> index {} -> value {}", o_A.x, o_A.y, index_o_A, output.data[index_o_A]);
        sirius::gdal::Save(output, sirius::tests::kOutputDirectoryPath + "point_black_rot+45.tif");
        REQUIRE(std::round(output.data[index_o_A]) == 13);

    }

    SECTION("lena - positive rotation +90 deg") {
        sirius::rotation::Parameters params{90};
        REQUIRE_NOTHROW(output = frequency_rot->Compute(lena_image, {}, params));
        LOG("tests", debug, "input size: {}, {}", lena_image.size.row,
            lena_image.size.col);
        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size.row == lena_image.size.col);
        REQUIRE(output.size.col == lena_image.size.row);
        // Lena : 256x256
        // Input point A {128,64} => A' = {64,128}
        sirius::Point i_A(128,64);
        sirius::Point o_A(64,256-128+1);
        int index_i_A = (i_A.y - 1)*lena_image.size.col + i_A.x - 1;
        int index_o_A = (o_A.y - 1)*output.size.col + o_A.x - 1;
        LOG("tests", debug, " input  A {},{} -> index {}", i_A.x, i_A.y, index_i_A);
        LOG("tests", debug, " output A {},{} -> index {}", o_A.x, o_A.y, index_o_A);
        sirius::gdal::Save(output, sirius::tests::kOutputDirectoryPath + "lena_rot+90.tif");
        REQUIRE(std::round(lena_image.data[index_i_A]) ==
                              std::round(output.data[index_o_A]));

    }

    SECTION("lena - positive rotation 45 deg") {
        sirius::rotation::Parameters params{45};
        REQUIRE_NOTHROW(output = frequency_rot->Compute(lena_image, {}, params));
        LOG("tests", debug, "input size: {}, {}", lena_image.size.row,
            lena_image.size.col);
        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size.row == 363);
        REQUIRE(output.size.col == 363);
        // Lena : 256x256
        sirius::Point o_A(182, 188);
        int index_o_A = (o_A.y - 1)*output.size.col + o_A.x - 1;
        sirius::gdal::Save(output, sirius::tests::kOutputDirectoryPath + "lena_rot+45.tif");
        LOG("tests", debug, " output A {},{} -> index {} -> value {}", o_A.x, o_A.y, index_o_A, output.data[index_o_A]);
        REQUIRE(std::round(output.data[index_o_A]) == 27);

    }

    SECTION("lena - negative rotation -45 deg") {
        sirius::rotation::Parameters params{-45};
        REQUIRE_NOTHROW(output = frequency_rot->Compute(lena_image, {}, params));
        LOG("tests", debug, "input size: {}, {}", lena_image.size.row,
            lena_image.size.col);
        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size.row == 363);
        REQUIRE(output.size.col == 363);
        // Lena : 256x256
        sirius::Point o_A(150, 130);
        int index_o_A = (o_A.y - 1)*output.size.col + o_A.x - 1;
        sirius::gdal::Save(output, sirius::tests::kOutputDirectoryPath + "lena_rot-45.tif");
        LOG("tests", debug, " output A {},{} -> index {} -> value {}", o_A.x, o_A.y, index_o_A, output.data[index_o_A]);
        REQUIRE(std::round(output.data[index_o_A]) == 154);

    }
}