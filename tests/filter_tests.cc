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

TEST_CASE("filter - empty filter", "[sirius]") {
    LOG_SET_LEVEL(trace);
    sirius::Size size = {256, 256};
    auto complex_array = sirius::fftw::CreateComplex(size);
    sirius::Filter filter;
    REQUIRE(!filter.IsLoaded());
    auto output = filter.Process(size, std::move(complex_array));
}

TEST_CASE("filter - dirac filter", "[sirius]") {
    LOG_SET_LEVEL(trace);
    sirius::Size size = {10, 10};
    auto zoom_ratio = sirius::ZoomRatio::Create(1, 1);
    auto complex_array = sirius::fftw::CreateComplex(size);
    std::memset(complex_array.get(), 1, size.CellCount());

    auto filter = sirius::Filter::Create(
          sirius::gdal::LoadImage("./filters/dirac_filter.tiff"), zoom_ratio);
    REQUIRE(filter.IsLoaded());

    auto output = filter.Process(size, std::move(complex_array));
}

TEST_CASE("filter - sinc2 filter", "[sirius]") {
    LOG_SET_LEVEL(trace);
    auto zoom_ratio = sirius::ZoomRatio::Create(2, 1);
    auto filter = sirius::Filter::Create(
          sirius::gdal::LoadImage("./filters/sinc_zoom2_filter.tif"),
          zoom_ratio);
    sirius::Size regular_size = {100, 100};
    sirius::Size large_size = {1000, 1000};

    SECTION("sinc2 filter - small image") {
        sirius::Size size{10, 10};
        auto complex_array = sirius::fftw::CreateComplex(size);
        std::memset(complex_array.get(), 1, size.CellCount());
        REQUIRE(filter.IsLoaded());
        REQUIRE_THROWS_AS(filter.Process(size, std::move(complex_array)),
                          sirius::Exception);
    }
    SECTION("sinc2 filter - regular image") {
        sirius::Size size{100, 100};
        auto complex_array = sirius::fftw::CreateComplex(size);
        std::memset(complex_array.get(), 1, size.CellCount());
        REQUIRE(filter.IsLoaded());
        REQUIRE_NOTHROW(complex_array =
                              filter.Process(size, std::move(complex_array)));
        REQUIRE_NOTHROW(filter.Process(size, std::move(complex_array)));
    }
    SECTION("sinc2 filter - large image") {
        sirius::Size size{1000, 1000};
        auto complex_array = sirius::fftw::CreateComplex(size);
        std::memset(complex_array.get(), 1, size.CellCount());
        REQUIRE(filter.IsLoaded());
        REQUIRE_NOTHROW(complex_array =
                              filter.Process(size, std::move(complex_array)));
        REQUIRE_NOTHROW(filter.Process(size, std::move(complex_array)));
    }
    SECTION("sinc2 filter - image size is filter size") {
        auto size = filter.size();
        auto complex_array = sirius::fftw::CreateComplex(size);
        std::memset(complex_array.get(), 1, size.CellCount());
        REQUIRE(filter.IsLoaded());
        REQUIRE_NOTHROW(complex_array =
                              filter.Process(size, std::move(complex_array)));
        REQUIRE_NOTHROW(filter.Process(size, std::move(complex_array)));
    }
}
