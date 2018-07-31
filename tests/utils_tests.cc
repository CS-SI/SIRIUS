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

#include <vector>

#include <catch/catch.hpp>

#include "sirius/types.h"

#include "sirius/utils/log.h"
#include "sirius/utils/lru_cache.h"
#include "sirius/utils/numeric.h"

TEST_CASE("utils tests - gcd", "[sirius]") {
    REQUIRE(sirius::utils::Gcd(0, 0) == 0);
    REQUIRE(sirius::utils::Gcd(1, 1) == 1);
    REQUIRE(sirius::utils::Gcd(2, 1) == 1);
    REQUIRE(sirius::utils::Gcd(2, 2) == 2);
    REQUIRE(sirius::utils::Gcd(4, 2) == 2);
    REQUIRE(sirius::utils::Gcd(50, 100) == 50);
}

TEST_CASE("utils tests - FFTShift2D", "[sirius]") {
    LOG_SET_LEVEL(trace);

    SECTION("FFTShift2D - even row, even col") {
        std::vector<double> input{11, 12, 13, 14, 21, 22, 23, 24,
                                  31, 32, 33, 34, 41, 42, 43, 44};
        std::vector<double> shifted_output(input.size());
        std::vector<double> output{33, 34, 31, 32, 43, 44, 41, 42,
                                   13, 14, 11, 12, 23, 24, 21, 22};

        sirius::utils::FFTShift2D(input.data(), {4, 4}, shifted_output.data());
        REQUIRE(std::equal(output.cbegin(), output.cend(),
                           shifted_output.cbegin()));
    }

    SECTION("FFTShift2D - even row, odd col") {
        std::vector<double> input{11, 12, 13, 14, 15, 21, 22, 23, 24, 25,
                                  31, 32, 33, 34, 35, 41, 42, 43, 44, 45};
        std::vector<double> shifted_output(input.size());
        std::vector<double> output{34, 35, 31, 32, 33, 44, 45, 41, 42, 43,
                                   14, 15, 11, 12, 13, 24, 25, 21, 22, 23};

        sirius::utils::FFTShift2D(input.data(), {4, 5}, shifted_output.data());
        REQUIRE(std::equal(output.cbegin(), output.cend(),
                           shifted_output.cbegin()));
    }

    SECTION("FFTShift2D - odd row, even col") {
        std::vector<double> input{11, 12, 13, 14, 21, 22, 23, 24, 31, 32,
                                  33, 34, 41, 42, 43, 44, 51, 52, 53, 54};
        std::vector<double> shifted_output(input.size());
        std::vector<double> output{43, 44, 41, 42, 53, 54, 51, 52, 13, 14,
                                   11, 12, 23, 24, 21, 22, 33, 34, 31, 32};

        sirius::utils::FFTShift2D(input.data(), {5, 4}, shifted_output.data());
        REQUIRE(std::equal(output.cbegin(), output.cend(),
                           shifted_output.cbegin()));
    }

    SECTION("FFTShift2D - odd row, odd col") {
        std::vector<double> input{11, 12, 13, 21, 22, 23, 31, 32, 33};
        std::vector<double> shifted_output(input.size());
        std::vector<double> output{33, 31, 32, 13, 11, 12, 23, 21, 22};

        sirius::utils::FFTShift2D(input.data(), {3, 3}, shifted_output.data());
        REQUIRE(std::equal(output.cbegin(), output.cend(),
                           shifted_output.cbegin()));
    }
}

TEST_CASE("utils tests - IFFTShift2D", "[sirius]") {
    LOG_SET_LEVEL(trace);

    SECTION("IFFTShift2D - even row, even col") {
        std::vector<double> input{11, 12, 13, 14, 21, 22, 23, 24,
                                  31, 32, 33, 34, 41, 42, 43, 44};
        std::vector<double> shifted_output(input.size());
        std::vector<double> output{33, 34, 31, 32, 43, 44, 41, 42,
                                   13, 14, 11, 12, 23, 24, 21, 22};

        sirius::utils::IFFTShift2D(input.data(), {4, 4}, shifted_output.data());
        REQUIRE(std::equal(output.cbegin(), output.cend(),
                           shifted_output.cbegin()));
    }

    SECTION("IFFTShift2D - even row, odd col") {
        std::vector<double> input{11, 12, 13, 14, 15, 21, 22, 23, 24, 25,
                                  31, 32, 33, 34, 35, 41, 42, 43, 44, 45};
        std::vector<double> shifted_output(input.size());
        std::vector<double> output{33, 34, 35, 31, 32, 43, 44, 45, 41, 42,
                                   13, 14, 15, 11, 12, 23, 24, 25, 21, 22};

        sirius::utils::IFFTShift2D(input.data(), {4, 5}, shifted_output.data());
        REQUIRE(std::equal(output.cbegin(), output.cend(),
                           shifted_output.cbegin()));
    }

    SECTION("IFFTShift2D - odd row, even col") {
        std::vector<double> input{11, 12, 13, 14, 21, 22, 23, 24, 31, 32,
                                  33, 34, 41, 42, 43, 44, 51, 52, 53, 54};
        std::vector<double> shifted_output(input.size());
        std::vector<double> output{33, 34, 31, 32, 43, 44, 41, 42, 53, 54,
                                   51, 52, 13, 14, 11, 12, 23, 24, 21, 22};

        sirius::utils::IFFTShift2D(input.data(), {5, 4}, shifted_output.data());
        REQUIRE(std::equal(output.cbegin(), output.cend(),
                           shifted_output.cbegin()));
    }

    SECTION("IFFTShift2D - odd row, odd col") {
        std::vector<double> input{1, 2, 3, 4, 5, 6, 7, 8, 9};
        std::vector<double> shifted_output(input.size());
        std::vector<double> output{5, 6, 4, 8, 9, 7, 2, 3, 1};

        sirius::utils::IFFTShift2D(input.data(), {3, 3}, shifted_output.data());
        REQUIRE(std::equal(output.cbegin(), output.cend(),
                           shifted_output.cbegin()));
    }
}

TEST_CASE("utils tests - IFFTShift2D(FFTShift2D)", "[sirius]") {
    LOG_SET_LEVEL(trace);

    SECTION("IFFTShift2D(FFTShift2D) - even row, even col") {
        std::vector<double> input{11, 12, 13, 14, 21, 22, 23, 24,
                                  31, 32, 33, 34, 41, 42, 43, 44};
        std::vector<double> shifted_output(input.size());
        std::vector<double> output(input.size());

        sirius::utils::FFTShift2D(input.data(), {4, 4}, shifted_output.data());
        sirius::utils::IFFTShift2D(shifted_output.data(), {4, 4},
                                   output.data());
        REQUIRE(std::equal(output.cbegin(), output.cend(), input.cbegin()));
    }

    SECTION("IFFTShift2D(FFTShift2D) - even row, odd col") {
        std::vector<double> input{11, 12, 13, 14, 15, 21, 22, 23, 24, 25,
                                  31, 32, 33, 34, 35, 41, 42, 43, 44, 45};
        std::vector<double> shifted_output(input.size());
        std::vector<double> output(input.size());

        sirius::utils::FFTShift2D(input.data(), {4, 5}, shifted_output.data());
        sirius::utils::IFFTShift2D(shifted_output.data(), {4, 5},
                                   output.data());
        REQUIRE(std::equal(output.cbegin(), output.cend(), input.cbegin()));
    }

    SECTION("IFFTShift2D(FFTShift2D) - odd row, even col") {
        std::vector<double> input{11, 12, 13, 14, 21, 22, 23, 24, 31, 32,
                                  33, 34, 41, 42, 43, 44, 51, 52, 53, 54};
        std::vector<double> shifted_output(input.size());
        std::vector<double> output(input.size());

        sirius::utils::FFTShift2D(input.data(), {5, 4}, shifted_output.data());
        sirius::utils::IFFTShift2D(shifted_output.data(), {5, 4},
                                   output.data());
        REQUIRE(std::equal(output.cbegin(), output.cend(), input.cbegin()));
    }

    SECTION("IFFTShift2D(FFTShift2D) - odd row, odd col") {
        std::vector<double> input{11, 12, 13, 21, 22, 23, 31, 32, 33};
        std::vector<double> shifted_output(input.size());
        std::vector<double> output(input.size());

        sirius::utils::FFTShift2D(input.data(), {3, 3}, shifted_output.data());
        sirius::utils::IFFTShift2D(shifted_output.data(), {3, 3},
                                   output.data());
        REQUIRE(std::equal(output.cbegin(), output.cend(), input.cbegin()));
    }
}

TEST_CASE("utils tests - IFFTShift2DUncentered", "[sirius]") {
    LOG_SET_LEVEL(trace);

    SECTION("IFFTShift2DUncentered - odd row, odd col") {
        std::vector<double> input{0, 1, 2, 3, 4, 5, 6, 7,
                                  8, 9, 10, 11, 12, 13, 14, 15,
                                  16, 17, 18, 19, 20, 21, 22, 23, 24};
        std::vector<double> shifted_output(input.size());
        std::vector<double> output{18, 19, 15, 16, 17, 23, 24, 20,
                                   21, 22, 3, 4, 0, 1, 2, 8, 9, 5, 6, 7,
                                   13, 14, 10, 11, 12};

        sirius::utils::IFFTShift2DUncentered(input.data(), {5, 5}, {3, 3}, shifted_output.data());
        REQUIRE(std::equal(output.cbegin(), output.cend(),
                           shifted_output.cbegin()));
    }
    
	SECTION("IFFTShift2DUncentered - odd row, odd col") {
        std::vector<double> input{0, 1, 2, 3, 4, 5, 6, 7,
                                  8, 9, 10, 11, 12, 13, 14, 15,
                                  16, 17, 18, 19, 20, 21, 22, 23, 24};
        std::vector<double> shifted_output(input.size());
        std::vector<double> output{24, 20, 21, 22, 23, 4, 0, 1,
                                   2, 3, 9, 5, 6, 7, 8, 14, 10, 11, 12, 13,
                                   19, 15, 16, 17, 18};

        sirius::utils::IFFTShift2DUncentered(input.data(), {5, 5}, {4, 4}, shifted_output.data());
        REQUIRE(std::equal(output.cbegin(), output.cend(),
                           shifted_output.cbegin()));
    }
    
    SECTION("IFFTShift2DUncentered - even row, even col") {
        std::vector<double> input{0, 1, 2, 3, 4, 5, 6, 7,
                                  8, 9, 10, 11, 12, 13, 14, 15,
                                 };
        std::vector<double> shifted_output(input.size());
        std::vector<double> output{5, 6, 7, 4, 9, 10, 11, 8, 13, 14, 15, 12, 1, 2, 3, 0};

        sirius::utils::IFFTShift2DUncentered(input.data(), {4, 4}, {1, 1}, shifted_output.data());
        REQUIRE(std::equal(output.cbegin(), output.cend(),
                           shifted_output.cbegin()));
    }
    
	SECTION("IFFTShift2DUncentered - even row, odd col") {
        std::vector<double> input{0, 1, 2, 3, 4, 5, 6, 7,
                                  8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
                                  18, 19
                                 };
        std::vector<double> shifted_output(input.size());
        std::vector<double> output{8, 9, 5, 6, 7, 13, 14, 10, 11, 12, 18, 19, 15, 16, 17,
								   3, 4, 0, 1, 2};

        sirius::utils::IFFTShift2DUncentered(input.data(), {4, 5}, {3, 1}, shifted_output.data());
        REQUIRE(std::equal(output.cbegin(), output.cend(),
                           shifted_output.cbegin()));
    }
    
    SECTION("IFFTShift2DUncentered - odd row, even col") {
        std::vector<double> input{0, 1, 2, 3, 4, 5, 6, 7,
                                  8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
                                  18, 19
                                 };
        std::vector<double> shifted_output(input.size());
        std::vector<double> output{1, 2, 3, 0, 5, 6, 7, 4, 9, 10, 11, 8, 13, 14, 15, 12, 17, 18, 19, 16};

        sirius::utils::IFFTShift2DUncentered(input.data(), {5, 4}, {1, 0}, shifted_output.data());
        REQUIRE(std::equal(output.cbegin(), output.cend(),
                           shifted_output.cbegin()));
    }
}

TEST_CASE("utils tests - FFTShift2DUncentered", "[sirius]") {
    LOG_SET_LEVEL(trace);

    SECTION("FFTShift2DUncentered - odd row, odd col") {
        std::vector<double> input{18, 19, 15, 16, 17, 23, 24, 20,
                                  21, 22, 3, 4, 0, 1, 2, 8, 9, 5, 6, 7,
                                  13, 14, 10, 11, 12,};
        std::vector<double> shifted_output(input.size());
        std::vector<double> output{0, 1, 2, 3, 4, 5, 6, 7,
								   8, 9, 10, 11, 12, 13, 14, 15,
                                   16, 17, 18, 19, 20, 21, 22, 23, 24};

        sirius::utils::FFTShift2DUncentered(input.data(), {5, 5}, {3, 3}, shifted_output.data());
        REQUIRE(std::equal(output.cbegin(), output.cend(),
                           shifted_output.cbegin()));
    }
    
	SECTION("FFTShift2DUncentered - odd row, odd col") {
        std::vector<double> input{24, 20, 21, 22, 23, 4, 0, 1,
                                  2, 3, 9, 5, 6, 7, 8, 14, 10, 11, 12, 13,
                                  19, 15, 16, 17, 18};
        std::vector<double> shifted_output(input.size());
        std::vector<double> output{0, 1, 2, 3, 4, 5, 6, 7,
                                   8, 9, 10, 11, 12, 13, 14, 15,
                                   16, 17, 18, 19, 20, 21, 22, 23, 24};

        sirius::utils::FFTShift2DUncentered(input.data(), {5, 5}, {4, 4}, shifted_output.data());
        REQUIRE(std::equal(output.cbegin(), output.cend(),
                           shifted_output.cbegin()));
    }
    
    SECTION("FFTShift2DUncentered - even row, even col") {
        std::vector<double> input{5, 6, 7, 4, 9, 10, 11, 8, 13, 14, 15, 12, 1, 2, 3, 0};
        std::vector<double> shifted_output(input.size());
        std::vector<double> output{ 0, 1, 2, 3, 4, 5, 6, 7,
                                    8, 9, 10, 11, 12, 13, 14, 15};

        sirius::utils::FFTShift2DUncentered(input.data(), {4, 4}, {1, 1}, shifted_output.data());
        REQUIRE(std::equal(output.cbegin(), output.cend(),
                           shifted_output.cbegin()));
    }
    
	SECTION("FFTShift2DUncentered - even row, odd col") {
        std::vector<double> input{8, 9, 5, 6, 7, 13, 14, 10, 11, 12, 18, 19, 15, 16, 17,
								  3, 4, 0, 1, 2
                                 };
        std::vector<double> shifted_output(input.size());
        std::vector<double> output{0, 1, 2, 3, 4, 5, 6, 7,
                                   8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
                                   18, 19};

        sirius::utils::FFTShift2DUncentered(input.data(), {4, 5}, {3, 1}, shifted_output.data());
        REQUIRE(std::equal(output.cbegin(), output.cend(),
                           shifted_output.cbegin()));
    }
    
    SECTION("FFTShift2DUncentered - odd row, even col") {
        std::vector<double> input{1, 2, 3, 0, 5, 6, 7, 4, 9, 10, 11, 8, 13,
								  14, 15, 12, 17, 18, 19, 16
                                 };
        std::vector<double> shifted_output(input.size());
        std::vector<double> output{0, 1, 2, 3, 4, 5, 6, 7,
                                   8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
                                   18, 19};

        sirius::utils::FFTShift2DUncentered(input.data(), {5, 4}, {1, 0}, shifted_output.data());
        REQUIRE(std::equal(output.cbegin(), output.cend(),
                           shifted_output.cbegin()));
    }
}

TEST_CASE("utils test - LRU cache", "[sirius]") {
    struct DummyStruct {
        DummyStruct() = default;
        DummyStruct(int val) : value(val) {}
        int value = 0;
    };

    sirius::utils::LRUCache<sirius::Size, DummyStruct, 3> cache;

    cache.Insert({1, 1}, {1});
    REQUIRE(cache.Size() == 1);
    REQUIRE(1 == cache.Get({1, 1}).value);

    cache.Insert({1, 1}, {2});
    REQUIRE(cache.Size() == 1);
    REQUIRE(cache.Contains({1, 1}));
    // key already exists but value should be replaced by fresher one
    REQUIRE(2 == cache.Get({1, 1}).value);

    cache.Remove({1, 1});
    REQUIRE(cache.Size() == 0);

    cache.Insert({1, 1}, {1});
    cache.Insert({2, 2}, {2});
    cache.Insert({3, 3}, {3});
    REQUIRE(cache.Size() == 3);
    REQUIRE(cache.Get({1, 1}).value == 1);
    REQUIRE(cache.Get({2, 2}).value == 2);
    REQUIRE(cache.Get({3, 3}).value == 3);

    cache.Insert({4, 4}, {4});
    REQUIRE(cache.Size() == 3);
    REQUIRE(!cache.Contains({1, 1}));

    cache.Get({2, 2});
    cache.Insert({5, 5}, {5});
    REQUIRE(cache.Contains({2, 2}));
    REQUIRE(!cache.Contains({3, 3}));

    cache.Clear();
    REQUIRE(cache.Size() == 0);
}

TEST_CASE("utils test - FFTFreq", "[sirius]") {
    std::vector<double> freq = sirius::utils::ComputeFFTFreq(5, false);
    REQUIRE(freq[0] == 0.0);
    REQUIRE(freq[1] == 0.2);
    REQUIRE(freq[2] == 0.4);
    REQUIRE(freq[3] == -0.4);
    REQUIRE(freq[4] == -0.2);

    freq = sirius::utils::ComputeFFTFreq(8);
    REQUIRE(freq[0] == 0.0);
    REQUIRE(freq[1] == 0.125);
    REQUIRE(freq[2] == 0.25);
    REQUIRE(freq[3] == 0.375);
}

TEST_CASE("utils test - Meshgrid", "[sirius]") {
    std::vector<int> xx, yy;
    sirius::utils::CreateMeshgrid(0, 3, 0, 3, xx, yy);

    REQUIRE(xx[0] == 0);
    REQUIRE(xx[1] == 1);
    REQUIRE(xx[2] == 2);
    REQUIRE(xx[3] == 0);
    REQUIRE(xx[4] == 1);
    REQUIRE(xx[5] == 2);
    REQUIRE(xx[6] == 0);
    REQUIRE(xx[7] == 1);
    REQUIRE(xx[8] == 2);

    REQUIRE(yy[0] == 0);
    REQUIRE(yy[1] == 0);
    REQUIRE(yy[2] == 0);
    REQUIRE(yy[3] == 1);
    REQUIRE(yy[4] == 1);
    REQUIRE(yy[5] == 1);
    REQUIRE(yy[6] == 2);
    REQUIRE(yy[7] == 2);
    REQUIRE(yy[8] == 2);

    xx.clear();
    yy.clear();
    sirius::utils::CreateMeshgrid(0, 3, 0, 4, xx, yy);
    REQUIRE(xx[0] == 0);
    REQUIRE(xx[1] == 1);
    REQUIRE(xx[2] == 2);
    REQUIRE(xx[3] == 3);
    REQUIRE(xx[4] == 0);
    REQUIRE(xx[5] == 1);
    REQUIRE(xx[6] == 2);
    REQUIRE(xx[7] == 3);
    REQUIRE(xx[8] == 0);
    REQUIRE(xx[9] == 1);
    REQUIRE(xx[10] == 2);
    REQUIRE(xx[11] == 3);

    REQUIRE(yy[0] == 0);
    REQUIRE(yy[1] == 0);
    REQUIRE(yy[2] == 0);
    REQUIRE(yy[3] == 0);
    REQUIRE(yy[4] == 1);
    REQUIRE(yy[5] == 1);
    REQUIRE(yy[6] == 1);
    REQUIRE(yy[7] == 1);
    REQUIRE(yy[8] == 2);
    REQUIRE(yy[9] == 2);
    REQUIRE(yy[10] == 2);
    REQUIRE(yy[11] == 2);
}
