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

#include "sirius/frequency_translator_factory.h"

#include "sirius/gdal/wrapper.h"

#include "sirius/utils/log.h"

#include "utils.h"

TEST_CASE("frequency translater - periodic smooth", "[sirius]") {
    LOG_SET_LEVEL(trace);

    auto freq_trans = sirius::FrequencyTranslatorFactory::Create(
          sirius::image_decomposition::Policies::kPeriodicSmooth);

    // test input
    auto lena_image = sirius::gdal::LoadImage(sirius::tests::kLenaImagePath);

    // output
    sirius::Image output;

    SECTION("lena - positive translation on both axis") {
        sirius::translation::Parameters params{50.0, 50.0};
        REQUIRE_NOTHROW(output = freq_trans->Compute(lena_image, {}, params));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size.row == lena_image.size.row - 50.0);
        REQUIRE(output.size.col == lena_image.size.col - 50.0);

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);
    }

    SECTION("lena - positive translation on x axis") {
        sirius::translation::Parameters params{0.0, 50.0};
        REQUIRE_NOTHROW(output = freq_trans->Compute(lena_image, {}, params));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size.row == lena_image.size.row);
        REQUIRE(output.size.col == lena_image.size.col - 50.0);
        REQUIRE(std::round(output.data[50]) == std::round(lena_image.data[50]));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);
    }
    SECTION("lena - positive translation on y axis") {
        sirius::translation::Parameters params{50.0, 0.0};
        REQUIRE_NOTHROW(output = freq_trans->Compute(lena_image, {}, params));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size.row == lena_image.size.row - 50.0);
        REQUIRE(output.size.col == lena_image.size.col);
        REQUIRE(std::round(output.data[50]) == std::round(lena_image.data[50]));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);
    }

    SECTION("lena - negative translation on x axis") {
        sirius::translation::Parameters params{0.0, -50.0};
        REQUIRE_NOTHROW(output = freq_trans->Compute(lena_image, {}, params));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size.row == lena_image.size.row);
        REQUIRE(output.size.col == lena_image.size.col - 50.0);
        REQUIRE(std::round(output.data[50]) ==
                std::round(lena_image.data[100]));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);
    }

    SECTION("lena - negative translation on y axis") {
        sirius::translation::Parameters params{-50.0, 0.0};
        REQUIRE_NOTHROW(output = freq_trans->Compute(lena_image, {}, params));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size.row == lena_image.size.row - 50.0);
        REQUIRE(output.size.col == lena_image.size.col);
        REQUIRE(std::round(output.data[50]) ==
                std::round(lena_image.data[50 * lena_image.size.row + 50]));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);
    }

    SECTION("lena - sub pixel translation") {
        sirius::translation::Parameters params{-50.72, 50.72};
        REQUIRE_NOTHROW(output = freq_trans->Compute(lena_image, {}, params));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size.row == lena_image.size.row - 51);
        REQUIRE(output.size.col == lena_image.size.col - 51);

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);
    }

    SECTION("lena - no translation") {
        sirius::translation::Parameters params{0.0, 0.0};
        REQUIRE_NOTHROW(output = freq_trans->Compute(lena_image, {}, params));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size.row == lena_image.size.row);
        REQUIRE(output.size.col == lena_image.size.col);
        REQUIRE(std::round(output.data[500]) ==
                std::round(lena_image.data[500]));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);
    }
}