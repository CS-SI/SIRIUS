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

#include <thread>

#include "sirius/filter.h"
#include "sirius/types.h"

#include "sirius/frequency_resampler_factory.h"
#include "sirius/i_frequency_resampler.h"

#include "sirius/gdal/image_streamer.h"
#include "sirius/gdal/resampling/input_stream.h"
#include "sirius/gdal/resampling/output_stream.h"
#include "sirius/gdal/wrapper.h"

#include "sirius/image_decomposition/policies.h"

#include "sirius/utils/log.h"

#include "utils.h"

TEST_CASE("image streamer - resampling", "[sirius]") {
    LOG_SET_LEVEL(info);

    auto ratio = sirius::ZoomRatio::Create(2, 1);
    auto filter = sirius::Filter::Create(
          sirius::gdal::LoadImage(sirius::tests::kSincZoom2FilterPath), ratio);

    sirius::resampling::Parameters resampling_parameters{ratio, filter.get()};
    auto resampler = sirius::FrequencyResamplerFactory::Create(
          sirius::image_decomposition::Policies::kPeriodicSmooth,
          sirius::FrequencyUpsamplingStrategies::kZeroPadding);

    sirius::gdal::ImageStreamer<sirius::IFrequencyResampler,
                                sirius::gdal::resampling::InputStream,
                                sirius::gdal::resampling::OutputStream>
          image_streamer("./input/disparity.png", "./output/streamer_tests.tif",
                         {256, 256}, true, resampling_parameters,
                         std::thread::hardware_concurrency());

    REQUIRE_NOTHROW(image_streamer.Stream(*resampler, resampling_parameters));
}