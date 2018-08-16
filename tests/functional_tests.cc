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

#include "sirius/gdal/wrapper.h"

#include "sirius/utils/log.h"

#include "utils.h"

void GenerateZoomedImage(const std::string& output_filepath,
                         const sirius::Image& image,
                         const sirius::ZoomRatio& zoom_ratio,
                         const sirius::Filter& filter,
                         const sirius::IFrequencyResampler& freq_resampler) {
    LOG("functional_tests", debug, "* generating {}", output_filepath);
    auto output_image =
          freq_resampler.Compute(zoom_ratio, image, filter.padding(), filter);
    std::stringstream ss;
    ss << sirius::tests::kOutputDirectoryPath << '/' << output_filepath;
    sirius::gdal::SaveImage(output_image, ss.str());
}

TEST_CASE("functional tests", "[sirius]") {
    LOG_SET_LEVEL(trace);

    sirius::ZoomRatio zoom_ratio_1_1;

    auto zoom_ratio_2_1 = sirius::ZoomRatio::Create(2, 1);
    auto zoom_ratio_3_1 = sirius::ZoomRatio::Create(3, 1);
    auto zoom_ratio_15_1 = sirius::ZoomRatio::Create(15, 1);
    auto zoom_ratio_7_4 = sirius::ZoomRatio::Create(7, 4);
    auto zoom_ratio_1_3 = sirius::ZoomRatio::Create(1, 3);
    auto zoom_ratio_210_120 = sirius::ZoomRatio::Create(210, 120);

    auto lena_image = sirius::gdal::LoadImage(sirius::tests::kLenaImagePath);
    auto lena_255_image = sirius::gdal::LoadImage("./input/lena-255x255.tif");
    auto disp0_image = sirius::gdal::LoadImage("./input/disparity.png");
    auto triangle_image = sirius::gdal::LoadImage("./input/triangle_10x10.tif");
    auto door_image = sirius::gdal::LoadImage("./input/door.tif");
    auto half_door_image = sirius::gdal::LoadImage("./input/half-door.tif");

    sirius::Filter no_filter;
    auto dirac_2_1_filter = sirius::Filter::Create(
          sirius::gdal::LoadImage(sirius::tests::kDiracFilterPath),
          zoom_ratio_2_1);
    auto sinc_2_1_filter = sirius::Filter::Create(
          sirius::gdal::LoadImage(sirius::tests::kSincZoom2FilterPath),
          zoom_ratio_2_1);

    auto regular_zero_padding_freq_resampler =
          sirius::FrequencyResamplerFactory::Create(
                sirius::ImageDecompositionPolicies::kRegular,
                sirius::FrequencyZoomStrategies::kZeroPadding);

    auto regular_periodization_freq_resampler =
          sirius::FrequencyResamplerFactory::Create(
                sirius::ImageDecompositionPolicies::kRegular,
                sirius::FrequencyZoomStrategies::kPeriodization);

    auto periodic_smooth_zero_padding_freq_resampler =
          sirius::FrequencyResamplerFactory::Create(
                sirius::ImageDecompositionPolicies::kPeriodicSmooth,
                sirius::FrequencyZoomStrategies::kZeroPadding);

    GenerateZoomedImage("TN1.tif", lena_image, zoom_ratio_1_1, no_filter,
                        *regular_zero_padding_freq_resampler);
    GenerateZoomedImage("TN2.tif", lena_image, zoom_ratio_2_1, no_filter,
                        *regular_zero_padding_freq_resampler);
    GenerateZoomedImage("TN3.tif", disp0_image, zoom_ratio_2_1, no_filter,
                        *regular_zero_padding_freq_resampler);
    GenerateZoomedImage("TN4.tif", disp0_image, zoom_ratio_2_1,
                        dirac_2_1_filter, *regular_zero_padding_freq_resampler);
    GenerateZoomedImage("TN5.tif", lena_image, zoom_ratio_2_1, dirac_2_1_filter,
                        *regular_zero_padding_freq_resampler);
    GenerateZoomedImage("TN6.tif", disp0_image, zoom_ratio_2_1, sinc_2_1_filter,
                        *regular_zero_padding_freq_resampler);
    GenerateZoomedImage("TN7.tif", disp0_image, zoom_ratio_2_1, sinc_2_1_filter,
                        *regular_zero_padding_freq_resampler);
    GenerateZoomedImage("TN8.tif", disp0_image, zoom_ratio_2_1, sinc_2_1_filter,
                        *regular_periodization_freq_resampler);
    GenerateZoomedImage("TN9.tif", triangle_image, zoom_ratio_3_1, no_filter,
                        *regular_zero_padding_freq_resampler);
    GenerateZoomedImage("TN10.tif", triangle_image, zoom_ratio_3_1, no_filter,
                        *periodic_smooth_zero_padding_freq_resampler);
    GenerateZoomedImage("TN11.tif", door_image, zoom_ratio_3_1, no_filter,
                        *regular_zero_padding_freq_resampler);
    GenerateZoomedImage("TN12.tif", door_image, zoom_ratio_3_1, no_filter,
                        *periodic_smooth_zero_padding_freq_resampler);
    GenerateZoomedImage("TN13.tif", half_door_image, zoom_ratio_3_1, no_filter,
                        *regular_zero_padding_freq_resampler);
    GenerateZoomedImage("TN14.tif", half_door_image, zoom_ratio_3_1, no_filter,
                        *periodic_smooth_zero_padding_freq_resampler);

    GenerateZoomedImage("TF1.tif", lena_image, zoom_ratio_15_1, no_filter,
                        *regular_zero_padding_freq_resampler);
    GenerateZoomedImage("TF2.tif", lena_255_image, zoom_ratio_2_1, no_filter,
                        *regular_zero_padding_freq_resampler);
    GenerateZoomedImage("TF3.tif", lena_image, zoom_ratio_7_4, no_filter,
                        *regular_zero_padding_freq_resampler);
    GenerateZoomedImage("TF4.tif", disp0_image, zoom_ratio_1_3, no_filter,
                        *regular_zero_padding_freq_resampler);
    GenerateZoomedImage("TF5.tif", triangle_image, zoom_ratio_210_120,
                        no_filter,
                        *periodic_smooth_zero_padding_freq_resampler);
}
