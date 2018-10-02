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

#include "sirius/exception.h"
#include "sirius/filter.h"
#include "sirius/types.h"

#include "sirius/fftw/wrapper.h"
#include "sirius/gdal/wrapper.h"

#include "sirius/utils/log.h"

#include "utils.h"

TEST_CASE("filter - dirac filter", "[sirius]") {
    sirius::Size size = {10, 10};

    auto zoom_ratio = sirius::ZoomRatio::Create(1, 1);
    auto complex_array = sirius::fftw::CreateComplex(size);
    std::memset(complex_array.get(), 1, size.CellCount());

    auto filter = sirius::Filter::Create(
          sirius::gdal::Load(sirius::tests::kDiracFilterPath), zoom_ratio);
    REQUIRE(filter != nullptr);

    auto output = filter->Process(size, std::move(complex_array));
}

TEST_CASE("filter - sinc2 filter", "[sirius]") {
    auto zoom_ratio = sirius::ZoomRatio::Create(2, 1);
    auto filter = sirius::Filter::Create(
          sirius::gdal::Load(sirius::tests::kSincZoom2FilterPath), zoom_ratio);
    REQUIRE(filter != nullptr);

    SECTION("sinc2 filter - small image") {
        sirius::Size small_size = {10, 10};
        auto complex_array = sirius::fftw::CreateComplex(small_size);
        std::memset(complex_array.get(), 1, small_size.CellCount());
        REQUIRE_THROWS_AS(filter->Process(small_size, std::move(complex_array)),
                          sirius::Exception);
    }
    SECTION("sinc2 filter - regular image") {
        sirius::Size regular_size = {100, 100};
        auto complex_array = sirius::fftw::CreateComplex(regular_size);
        std::memset(complex_array.get(), 1, regular_size.CellCount());
        REQUIRE_NOTHROW(complex_array = filter->Process(
                              regular_size, std::move(complex_array)));
        REQUIRE_NOTHROW(
              filter->Process(regular_size, std::move(complex_array)));
    }
    SECTION("sinc2 filter - large image") {
        sirius::Size large_size = {1000, 1000};
        auto complex_array = sirius::fftw::CreateComplex(large_size);
        std::memset(complex_array.get(), 1, large_size.CellCount());
        REQUIRE_NOTHROW(complex_array = filter->Process(
                              large_size, std::move(complex_array)));
        REQUIRE_NOTHROW(filter->Process(large_size, std::move(complex_array)));
    }
    SECTION("sinc2 filter - image size is filter size") {
        auto size = filter->size();
        auto complex_array = sirius::fftw::CreateComplex(size);
        std::memset(complex_array.get(), 1, size.CellCount());
        REQUIRE_NOTHROW(complex_array =
                              filter->Process(size, std::move(complex_array)));
        REQUIRE_NOTHROW(filter->Process(size, std::move(complex_array)));
    }
}
