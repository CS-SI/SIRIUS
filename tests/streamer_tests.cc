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
#include "sirius/frequency_translator_factory.h"
#include "sirius/i_frequency_resampler.h"
#include "sirius/i_frequency_translator.h"

#include "sirius/gdal/image_streamer.h"
#include "sirius/gdal/resampling/input_stream.h"
#include "sirius/gdal/resampling/output_stream.h"
#include "sirius/gdal/translation/input_stream.h"
#include "sirius/gdal/translation/output_stream.h"

#include "sirius/gdal/wrapper.h"

#include "sirius/image_decomposition/policies.h"

#include "sirius/utils/log.h"

#include "utils.h"

TEST_CASE("image streamer - resampling", "[sirius]") {
    LOG_SET_LEVEL(info);

    const char kStreamResamplingInputFile[] =
          "./stream_resampling_input.tif";
    const char kStreamResamplingOutputFile[] =
          "./stream_resampling_output.tif";
    auto dummy_image = sirius::tests::CreateDummyImage({1024, 768});
    sirius::gdal::Save(dummy_image, kStreamResamplingInputFile);

    auto ratio = sirius::ZoomRatio::Create(2, 1);
    auto filter = sirius::Filter::Create(
          sirius::gdal::Load(sirius::tests::kSincZoom2FilterPath), ratio);

    sirius::resampling::Parameters resampling_parameters{ratio, filter.get()};
    auto resampler = sirius::FrequencyResamplerFactory::Create(
          sirius::image_decomposition::Policies::kPeriodicSmooth,
          sirius::FrequencyUpsamplingStrategies::kZeroPadding);

    sirius::gdal::ImageStreamer<sirius::IFrequencyResampler,
                                sirius::gdal::resampling::InputStream,
                                sirius::gdal::resampling::OutputStream>
          image_streamer(kStreamResamplingInputFile,
                         kStreamResamplingOutputFile, {256, 256}, true,
                         resampling_parameters,
                         std::thread::hardware_concurrency());

    REQUIRE_NOTHROW(image_streamer.Stream(*resampler, resampling_parameters));
}

TEST_CASE("image streamer - translation", "[sirius]") {
    LOG_SET_LEVEL(info);
    const char kStreamTranslationInputFile[] =
          "./output/stream_translation_input.tif";
    const char kStreamTranslationOutputFile[] =
          "./output/stream_translation_output.tif";
    auto dummy_image = sirius::tests::CreateSquaredImage({2048, 1536});
    sirius::gdal::Save(dummy_image, kStreamTranslationInputFile);

    sirius::translation::Parameters translation_parameters{50.5, 50.5};

    auto translator = sirius::FrequencyTranslatorFactory::Create(
          sirius::image_decomposition::Policies::kPeriodicSmooth);

    sirius::gdal::ImageStreamer<sirius::IFrequencyTranslator,
                                sirius::gdal::translation::InputStream,
                                sirius::gdal::translation::OutputStream>
          image_streamer(kStreamTranslationInputFile,
                         kStreamTranslationOutputFile, {256, 256}, true,
                         translation_parameters,
                         std::thread::hardware_concurrency());

    REQUIRE_NOTHROW(image_streamer.Stream(*translator, translation_parameters));
}
