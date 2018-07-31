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

#include "sirius/types.h"

TEST_CASE("types tests - ZoomRatio from int", "[sirius]") {
    REQUIRE_NOTHROW(sirius::ZoomRatio::Create(1, 1));
    REQUIRE_NOTHROW(sirius::ZoomRatio::Create(2, 1));
    REQUIRE_NOTHROW(sirius::ZoomRatio::Create(2));
    REQUIRE_NOTHROW(sirius::ZoomRatio::Create(14, 10));

    REQUIRE_THROWS(sirius::ZoomRatio::Create(-1));
    REQUIRE_THROWS(sirius::ZoomRatio::Create(1, -1));
    REQUIRE_THROWS(sirius::ZoomRatio::Create(0, 1));
    REQUIRE_THROWS(sirius::ZoomRatio::Create(1, 0));
}

TEST_CASE("types tests - ZoomRatio from string", "[sirius]") {
    REQUIRE_NOTHROW(sirius::ZoomRatio::Create("1:1"));
    REQUIRE_NOTHROW(sirius::ZoomRatio::Create("2:1"));
    REQUIRE_NOTHROW(sirius::ZoomRatio::Create("2"));
    REQUIRE_NOTHROW(sirius::ZoomRatio::Create("14:10"));

    REQUIRE_THROWS(sirius::ZoomRatio::Create(":"));
    REQUIRE_THROWS(sirius::ZoomRatio::Create("-"));
    REQUIRE_THROWS(sirius::ZoomRatio::Create(":1"));
    REQUIRE_THROWS(sirius::ZoomRatio::Create("1:"));
    REQUIRE_THROWS(sirius::ZoomRatio::Create("-:"));
    REQUIRE_THROWS(sirius::ZoomRatio::Create("-:-"));
    REQUIRE_THROWS(sirius::ZoomRatio::Create("-2:1"));
    REQUIRE_THROWS(sirius::ZoomRatio::Create("2:-1"));
}