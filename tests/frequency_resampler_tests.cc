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

#include <sstream>
#include <string>

#include <catch/catch.hpp>

#include "sirius/filter.h"
#include "sirius/image.h"

#include "sirius/frequency_resampler_factory.h"

#include "sirius/gdal/exception.h"
#include "sirius/gdal/wrapper.h"

#include "sirius/utils/log.h"

#include "utils.h"

TEST_CASE("frequency resampler - factory", "[sirius]") {
    auto classic_zero_padding_resampler =
          sirius::FrequencyResamplerFactory::Create(
                sirius::ImageDecompositionPolicies::kRegular,
                sirius::FrequencyZoomStrategies::kZeroPadding);
    REQUIRE(classic_zero_padding_resampler != nullptr);

    auto classic_periodization_resampler =
          sirius::FrequencyResamplerFactory::Create(
                sirius::ImageDecompositionPolicies::kRegular,
                sirius::FrequencyZoomStrategies::kPeriodization);
    REQUIRE(classic_periodization_resampler != nullptr);

    auto ps_zero_padding_resampler = sirius::FrequencyResamplerFactory::Create(
          sirius::ImageDecompositionPolicies::kPeriodicSmooth,
          sirius::FrequencyZoomStrategies::kZeroPadding);
    REQUIRE(ps_zero_padding_resampler != nullptr);

    auto ps_periodization_resampler = sirius::FrequencyResamplerFactory::Create(
          sirius::ImageDecompositionPolicies::kPeriodicSmooth,
          sirius::FrequencyZoomStrategies::kPeriodization);
    REQUIRE(ps_periodization_resampler != nullptr);
}

TEST_CASE("frequency resampler - classic decomposition - zero padding zoom",
          "[sirius]") {
    LOG_SET_LEVEL(trace);

    auto zoom_ratio = sirius::ZoomRatio::Create(2, 1);

    // test inputs
    auto dummy_image = sirius::tests::CreateDummyImage({20, 20});
    auto lena_image = sirius::gdal::LoadImage("./input/lena.jpg");
    auto disp0_image = sirius::gdal::LoadImage("./input/disparity.png");

    // filters
    auto dirac_filter = sirius::Filter::Create(
          sirius::gdal::LoadImage("./filters/dirac_filter.tiff"), zoom_ratio);
    auto sinc_zoom2_filter = sirius::Filter::Create(
          sirius::gdal::LoadImage("./filters/sinc_zoom2_filter.tif"),
          zoom_ratio);

    // output
    sirius::Image output;

    auto freq_resampler = sirius::FrequencyResamplerFactory::Create(
          sirius::ImageDecompositionPolicies::kRegular,
          sirius::FrequencyZoomStrategies::kZeroPadding);

    SECTION("dummy image - no filter") {
        REQUIRE_NOTHROW(
              output = freq_resampler->Compute(zoom_ratio, dummy_image, {}));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size ==
                (dummy_image.size * zoom_ratio.input_resolution()));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);
    }

    SECTION("dummy image - dirac filter") {
        REQUIRE_NOTHROW(output = freq_resampler->Compute(
                              zoom_ratio, dummy_image, dirac_filter.padding(),
                              dirac_filter));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size ==
                (dummy_image.size * zoom_ratio.input_resolution()));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);
    }

    SECTION("dummy image - sinc filter") {
        REQUIRE_NOTHROW(output = freq_resampler->Compute(
                              zoom_ratio, dummy_image,
                              sinc_zoom2_filter.padding(), sinc_zoom2_filter));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size ==
                (dummy_image.size * zoom_ratio.input_resolution()));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);
    }

    SECTION("Lena - no filter") {
        sirius::Image output;
        REQUIRE_NOTHROW(
              output = freq_resampler->Compute(zoom_ratio, lena_image, {}));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size ==
                (lena_image.size * zoom_ratio.input_resolution()));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);

        sirius::gdal::SaveImage(output, "./output/lena_no_filter.tif");
    }

    SECTION("Lena - dirac filter") {
        sirius::Image output;
        REQUIRE_NOTHROW(output = freq_resampler->Compute(zoom_ratio, lena_image,
                                                         dirac_filter.padding(),
                                                         dirac_filter));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size ==
                (lena_image.size * zoom_ratio.input_resolution()));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);

        sirius::gdal::SaveImage(output, "./output/lena_dirac_filter.tif");
    }

    SECTION("Lena - sinc filter") {
        sirius::Image output;
        REQUIRE_NOTHROW(output = freq_resampler->Compute(
                              zoom_ratio, lena_image,
                              sinc_zoom2_filter.padding(), sinc_zoom2_filter));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size ==
                (lena_image.size * zoom_ratio.input_resolution()));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);

        sirius::gdal::SaveImage(output, "./output/lena_sinc_filter.tif");
    }

    SECTION("disp0 - no filter") {
        sirius::Image output;
        REQUIRE_NOTHROW(
              output = freq_resampler->Compute(zoom_ratio, disp0_image, {}));
        REQUIRE(output.data.size() > 0);

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);

        sirius::gdal::SaveImage(output, "./output/disparity_no_filter.tif");
    }

    SECTION("disp0 - dirac") {
        sirius::Image output;
        REQUIRE_NOTHROW(output = freq_resampler->Compute(
                              zoom_ratio, disp0_image, dirac_filter.padding(),
                              dirac_filter));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size ==
                (disp0_image.size * zoom_ratio.input_resolution()));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);

        sirius::gdal::SaveImage(output, "./output/disparity_dirac_filter.tif");
    }

    SECTION("disp0 - sinc") {
        sirius::Image output;
        REQUIRE_NOTHROW(output = freq_resampler->Compute(
                              zoom_ratio, disp0_image,
                              sinc_zoom2_filter.padding(), sinc_zoom2_filter));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size ==
                (disp0_image.size * zoom_ratio.input_resolution()));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);

        sirius::gdal::SaveImage(output, "./output/disparity_sinc_filter.tif");
    }
}

TEST_CASE("frequency resampler - classic - periodization zoom", "[sirius]") {
    LOG_SET_LEVEL(trace);

    auto zoom_ratio = sirius::ZoomRatio::Create(2, 1);

    // test input
    auto dummy_image = sirius::tests::CreateDummyImage({4, 4});
    auto lena_image = sirius::gdal::LoadImage("./input/lena.jpg");
    auto disp0_image = sirius::gdal::LoadImage("./input/disparity.png");

    // filters
    auto dirac_filter = sirius::Filter::Create(
          sirius::gdal::LoadImage("./filters/dirac_filter.tiff"), zoom_ratio);
    auto sinc_zoom2_filter = sirius::Filter::Create(
          sirius::gdal::LoadImage("./filters/sinc_zoom2_filter.tif"),
          zoom_ratio);

    // output
    sirius::Image output;

    auto freq_resampler = sirius::FrequencyResamplerFactory::Create(
          sirius::ImageDecompositionPolicies::kRegular,
          sirius::FrequencyZoomStrategies::kPeriodization);

    SECTION("dummy image - no filter") {
        REQUIRE_NOTHROW(
              output = freq_resampler->Compute(zoom_ratio, dummy_image, {}));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size ==
                (dummy_image.size * zoom_ratio.input_resolution()));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);
    }

    SECTION("dummy image - dirac filter") {
        REQUIRE_NOTHROW(output = freq_resampler->Compute(
                              zoom_ratio, dummy_image, dirac_filter.padding(),
                              dirac_filter));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size ==
                (dummy_image.size * zoom_ratio.input_resolution()));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);
    }

    SECTION("dummy image - sinc filter") {
        REQUIRE_NOTHROW(output = freq_resampler->Compute(
                              zoom_ratio, dummy_image,
                              sinc_zoom2_filter.padding(), sinc_zoom2_filter));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size ==
                (dummy_image.size * zoom_ratio.input_resolution()));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);
    }

    SECTION("Lena - no filter") {
        sirius::Image output;
        REQUIRE_NOTHROW(
              output = freq_resampler->Compute(zoom_ratio, lena_image, {}));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size ==
                (lena_image.size * zoom_ratio.input_resolution()));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);

        sirius::gdal::SaveImage(output,
                                "./output/lena_periodization_no_filter.tif");
    }

    SECTION("Lena - dirac filter") {
        sirius::Image output;
        REQUIRE_NOTHROW(output = freq_resampler->Compute(zoom_ratio, lena_image,
                                                         dirac_filter.padding(),
                                                         dirac_filter));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size ==
                (lena_image.size * zoom_ratio.input_resolution()));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);

        sirius::gdal::SaveImage(output,
                                "./output/lena_periodization_dirac_filter.tif");
    }

    SECTION("Lena - sinc filter") {
        sirius::Image output;
        REQUIRE_NOTHROW(output = freq_resampler->Compute(
                              zoom_ratio, lena_image,
                              sinc_zoom2_filter.padding(), sinc_zoom2_filter));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size ==
                (lena_image.size * zoom_ratio.input_resolution()));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);

        sirius::gdal::SaveImage(output,
                                "./output/lena_periodization_sinc_filter.tif");
    }

    SECTION("disp0 - no filter") {
        sirius::Image output;
        REQUIRE_NOTHROW(
              output = freq_resampler->Compute(zoom_ratio, disp0_image, {}));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size ==
                (disp0_image.size * zoom_ratio.input_resolution()));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);

        sirius::gdal::SaveImage(
              output, "./output/disparity_periodization_no_filter.tif");
    }

    SECTION("disp0 - dirac") {
        sirius::Image output;
        REQUIRE_NOTHROW(output = freq_resampler->Compute(
                              zoom_ratio, disp0_image, dirac_filter.padding(),
                              dirac_filter));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size ==
                (disp0_image.size * zoom_ratio.input_resolution()));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);

        sirius::gdal::SaveImage(
              output, "./output/disparity_periodization_dirac_filter.tif");
    }

    SECTION("disp0 - sinc") {
        sirius::Image output;
        REQUIRE_NOTHROW(output = freq_resampler->Compute(
                              zoom_ratio, disp0_image,
                              sinc_zoom2_filter.padding(), sinc_zoom2_filter));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size ==
                (disp0_image.size * zoom_ratio.input_resolution()));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);

        sirius::gdal::SaveImage(
              output, "./output/disparity_periodization_sinc_filter.tif");
    }
}

TEST_CASE("frequency resampler - periodic smooth - zero padding", "[sirius]") {
    LOG_SET_LEVEL(trace);

    auto zoom_ratio = sirius::ZoomRatio::Create(2, 1);

    // test input
    auto dummy_image = sirius::tests::CreateDummyImage({256, 256});
    auto lena_image = sirius::gdal::LoadImage("./input/lena.jpg");
    auto disp0_image = sirius::gdal::LoadImage("./input/disparity.png");

    // output
    sirius::Image output;

    auto freq_resampler = sirius::FrequencyResamplerFactory::Create(
          sirius::ImageDecompositionPolicies::kPeriodicSmooth,
          sirius::FrequencyZoomStrategies::kZeroPadding);

    SECTION("dummy image - no filter") {
        REQUIRE_NOTHROW(
              output = freq_resampler->Compute(zoom_ratio, dummy_image, {}));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size ==
                (dummy_image.size * zoom_ratio.input_resolution()));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);
    }

    SECTION("Lena - no filter") {
        sirius::Image output;
        REQUIRE_NOTHROW(
              output = freq_resampler->Compute(zoom_ratio, lena_image, {}));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size ==
                (lena_image.size * zoom_ratio.input_resolution()));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);

        sirius::gdal::SaveImage(output,
                                "./output/lena_periodic_smooth_no_filter.tif");
    }

    SECTION("disp0 - no filter") {
        sirius::Image output;
        REQUIRE_NOTHROW(
              output = freq_resampler->Compute(zoom_ratio, disp0_image, {}));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size ==
                (disp0_image.size * zoom_ratio.input_resolution()));

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);

        sirius::gdal::SaveImage(
              output, "./output/disparity_periodic_smooth_no_filter.tif");
    }
}

TEST_CASE("frequency resampler - real zoom", "[sirius]") {
    LOG_SET_LEVEL(trace);

    auto zoom_ratio = sirius::ZoomRatio::Create(7, 4);

    // test input
    auto dummy_image = sirius::tests::CreateDummyImage({256, 256});
    auto lena_image = sirius::gdal::LoadImage("./input/lena.jpg");
    auto disp0_image = sirius::gdal::LoadImage("./input/disparity.png");

    // output
    sirius::Image output;

    auto freq_resampler = sirius::FrequencyResamplerFactory::Create(
          sirius::ImageDecompositionPolicies::kRegular,
          sirius::FrequencyZoomStrategies::kZeroPadding);

    SECTION("dummy image - no filter") {
        REQUIRE_NOTHROW(
              output = freq_resampler->Compute(zoom_ratio, dummy_image, {}));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size == dummy_image.size * zoom_ratio.ratio());

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);
    }

    SECTION("Lena - no filter") {
        sirius::Image output;
        REQUIRE_NOTHROW(
              output = freq_resampler->Compute(zoom_ratio, lena_image, {}));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size == lena_image.size * zoom_ratio.ratio());

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);

        sirius::gdal::SaveImage(output,
                                "./output/lena_unzoom_7_4_no_filter.tif");
    }

    /*SECTION("disp0 - no filter") {
        sirius::Image output;
        REQUIRE_NOTHROW(
              output = freq_resampler->Compute(zoom_ratio, disp0_image, {}));
        REQUIRE(output.data.size() > 0);
        REQUIRE(output.size == disp0_image.size * zoom_ratio.ratio());

        LOG("tests", debug, "output size: {}, {}", output.size.row,
            output.size.col);

        sirius::gdal::SaveImage(output,
                                "./output/disparity_unzoom_7_4_no_filter.tif");
    }*/
}

TEST_CASE("frequency resampler - example", "[sirius]") {
    LOG_SET_LEVEL(trace);

    auto zoom_ratio_2_1 = sirius::ZoomRatio::Create(2, 1);
    auto zoom_ratio_7_4 = sirius::ZoomRatio::Create(7, 4);

    sirius::Image image = sirius::tests::CreateDummyImage({256, 256});

    auto dirac_filter = sirius::Filter::Create(
          sirius::gdal::LoadImage("./filters/dirac_filter.tiff"),
          zoom_ratio_7_4);
    auto sinc_filter = sirius::Filter::Create(
          sirius::gdal::LoadImage("./filters/sinc_zoom2_filter.tif"),
          zoom_ratio_2_1);

    sirius::IFrequencyResampler::UPtr freq_resampler =
          sirius::FrequencyResamplerFactory::Create(
                sirius::ImageDecompositionPolicies::kRegular,
                sirius::FrequencyZoomStrategies::kZeroPadding);

    sirius::Image zoomed_image_7_4 = freq_resampler->Compute(
          zoom_ratio_7_4, image, dirac_filter.padding(), dirac_filter);

    sirius::Image zoomed_image_2_1 = freq_resampler->Compute(
          zoom_ratio_2_1, image, sinc_filter.padding(), sinc_filter);
}
