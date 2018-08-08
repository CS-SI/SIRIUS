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

#include <exception>
#include <future>
#include <iostream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

#include <cxxopts.hpp>

#include "sirius/exception.h"
#include "sirius/frequency_resampler_factory.h"
#include "sirius/image_streamer.h"
#include "sirius/sirius.h"

#include "sirius/gdal/wrapper.h"

#include "sirius/utils/log.h"
#include "sirius/utils/numeric.h"

struct CliParameters {
    // status
    bool parsed = false;
    bool help_requested = false;
    std::string help_message;

    // mandatory arguments
    std::string input_image_path;
    std::string output_image_path;

    // general options
    std::string verbosity_level = "info";

    // resampling options
    std::string resampling_ratio = "1:1";
    bool no_image_decomposition = false;
    bool upsample_periodization = false;
    bool upsample_zero_padding = false;

    // filter options
    std::string filter_path;
    bool zero_pad_real_edges = false;

    // stream mode options
    bool stream_mode = false;
    int stream_block_height = 256;
    int stream_block_width = 256;
    bool stream_no_block_resizing = false;
    bool filter_normalize = false;
    int hot_point_x = -1;
    int hot_point_y = -1;
    unsigned int stream_parallel_workers = std::thread::hardware_concurrency();

    bool HasStreamMode() const {
        return stream_mode && stream_block_height > 0 && stream_block_width > 0;
    }

    sirius::Size GetStreamBlockSize() const {
        return {stream_block_height, stream_block_width};
    }
};

CliParameters GetCliParameters(int argc, const char* argv[]);
void RunRegularMode(const sirius::IFrequencyResampler& frequency_resampler,
                    const sirius::Filter& filter,
                    const sirius::ZoomRatio& zoom_ratio,
                    const CliParameters& params);
void RunStreamMode(const sirius::IFrequencyResampler& frequency_resampler,
                   const sirius::Filter& filter,
                   const sirius::ZoomRatio& zoom_ratio,
                   const CliParameters& params);

int main(int argc, const char* argv[]) {
    CliParameters params = GetCliParameters(argc, argv);
    if (params.help_requested || !params.parsed) {
        std::cerr << params.help_message;
        return params.help_requested ? 0 : 1;
    }

    if (params.input_image_path.empty() || params.output_image_path.empty()) {
        std::cerr << params.help_message << std::endl;
        std::cerr << "sirius: input and/or output arguments are missing"
                  << std::endl;
        return 1;
    }

    sirius::utils::SetVerbosityLevel(params.verbosity_level);

    LOG("sirius", info, "Sirius {} - {}", sirius::kVersion, sirius::kGitCommit);

    try {
        auto zoom_ratio = sirius::ZoomRatio::Create(params.resampling_ratio);
        LOG("sirius", info, "resampling ratio: {}:{}",
            zoom_ratio.input_resolution(), zoom_ratio.output_resolution());

        // filter parameters
        sirius::PaddingType padding_type = sirius::PaddingType::kMirrorPadding;
        if (params.zero_pad_real_edges) {
            LOG("sirius", info, "filter: border zero padding");
            padding_type = sirius::PaddingType::kZeroPadding;
        } else {
            LOG("sirius", info, "filter: mirror padding");
        }

        if (params.filter_normalize) {
            LOG("sirius", info, "filter: normalize");
        }

        sirius::Filter filter;
        if (!params.filter_path.empty()) {
            LOG("sirius", info, "filter path: {}", params.filter_path);
            sirius::Point hp(params.hot_point_x, params.hot_point_y);
            filter = sirius::Filter::Create(
                  sirius::gdal::LoadImage(params.filter_path), zoom_ratio, hp,
                  padding_type, params.filter_normalize);
        }

        // resampling parameters
        sirius::ImageDecompositionPolicies image_decomposition_policy =
              sirius::ImageDecompositionPolicies::kPeriodicSmooth;
        sirius::FrequencyZoomStrategies zoom_strategy =
              sirius::FrequencyZoomStrategies::kPeriodization;

        if (params.no_image_decomposition) {
            LOG("sirius", info, "image decomposition: none");
            image_decomposition_policy =
                  sirius::ImageDecompositionPolicies::kRegular;
        } else {
            LOG("sirius", info, "image decomposition: periodic plus smooth");
        }

        if (zoom_ratio.ratio() > 1) {
            // choose the upsampling algorithm only if ratio > 1
            if (params.upsample_periodization && !filter.IsLoaded()) {
                LOG("sirius", error,
                    "filter is required with periodization upsampling");
                return 1;
            } else if (params.upsample_zero_padding || !filter.IsLoaded()) {
                LOG("sirius", info, "upsampling: zero padding");
                zoom_strategy = sirius::FrequencyZoomStrategies::kZeroPadding;
                if (filter.IsLoaded()) {
                    LOG("sirius", warn,
                        "filter will be used with zero padding upsampling");
                }
            } else {
                LOG("sirius", info, "upsampling: periodization");
                zoom_strategy = sirius::FrequencyZoomStrategies::kPeriodization;
            }
        }

        auto frequency_resampler = sirius::FrequencyResamplerFactory::Create(
              image_decomposition_policy, zoom_strategy);

        if (!params.HasStreamMode()) {
            RunRegularMode(*frequency_resampler, filter, zoom_ratio, params);
        } else {
            RunStreamMode(*frequency_resampler, filter, zoom_ratio, params);
        }
    } catch (const std::exception& e) {
        std::cerr << "sirius: exception while computing resampling: "
                  << e.what() << std::endl;
        return 1;
    }

    return 0;
}

void RunRegularMode(const sirius::IFrequencyResampler& frequency_resampler,
                    const sirius::Filter& filter,
                    const sirius::ZoomRatio& zoom_ratio,
                    const CliParameters& params) {
    LOG("sirius", info, "regular mode");
    auto input_image = sirius::gdal::LoadImage(params.input_image_path);
    LOG("sirius", info, "input image '{}' ({}x{})", params.input_image_path,
        input_image.size.row, input_image.size.col);

    auto resampled_geo_ref = sirius::gdal::ComputeResampledGeoReference(
          params.input_image_path, zoom_ratio);

    auto resampled_image = frequency_resampler.Compute(
          zoom_ratio, input_image, filter.padding(), filter);
    LOG("sirius", info, "resampled image '{}' ({}x{})",
        params.output_image_path, resampled_image.size.row,
        resampled_image.size.col);
    sirius::gdal::SaveImage(resampled_image, params.output_image_path,
                            resampled_geo_ref);
}

void RunStreamMode(const sirius::IFrequencyResampler& frequency_resampler,
                   const sirius::Filter& filter,
                   const sirius::ZoomRatio& zoom_ratio,
                   const CliParameters& params) {
    LOG("sirius", info, "streaming mode");
    unsigned int max_parallel_workers =
          std::max(std::min(params.stream_parallel_workers,
                            std::thread::hardware_concurrency()),
                   1u);
    auto stream_block_size = params.GetStreamBlockSize();

    // improve stream_block_size if requested or required
    if (!zoom_ratio.IsRealZoom()) {
        if (!params.stream_no_block_resizing) {
            stream_block_size = sirius::utils::GenerateDyadicSize(
                  stream_block_size, zoom_ratio.input_resolution(),
                  filter.padding_size());
            LOG("sirius", warn, "stream block resized to dyadic size: {}x{}",
                stream_block_size.row, stream_block_size.col);
        }
    } else {
        // real zoom needs specific block size (row and col should be multiple
        // of input resolution and output resolution)
        stream_block_size = sirius::utils::GenerateZoomCompliantSize(
              stream_block_size, zoom_ratio);
        LOG("sirius", warn, "stream block resized to comply with zoom: {}x{}",
            stream_block_size.row, stream_block_size.col);
    }
    sirius::ImageStreamer streamer(
          params.input_image_path, params.output_image_path, stream_block_size,
          zoom_ratio, filter.Metadata(), max_parallel_workers);
    streamer.Stream(frequency_resampler, filter);
}

CliParameters GetCliParameters(int argc, const char* argv[]) {
    CliParameters params;
    std::stringstream description;
    description << "Sirius " << sirius::kVersion << " (" << sirius::kGitCommit
                << ")" << std::endl
                << "Standalone tool to resample and filter images in the "
                   "frequency domain"
                << std::endl;
    cxxopts::Options options(argv[0], description.str());
    options.positional_help("input-image output-image").show_positional_help();

    auto max_threads_string =
          std::to_string(std::thread::hardware_concurrency());
    std::stringstream stream_parallel_workers_desc;
    stream_parallel_workers_desc
          << "Parallel workers used to compute resampling ("
          << max_threads_string << " max)";

    // clang-format off
    options.add_options("")
        ("h,help", "Show help")
        ("v,verbosity",
         "Set verbosity level (trace,debug,info,warn,err,critical,off)",
         cxxopts::value(params.verbosity_level)->default_value("info"));

    options.add_options("resampling")
        ("r,resampling-ratio", "Resampling ratio as input:output, "
          "allowed format: I (equivalent to I:1), I:O",
         cxxopts::value(params.resampling_ratio)->default_value("1:1"))
        ("no-image-decomposition",
         "Do not decompose the input image "
         "(default: periodic plus smooth image decomposition)",
         cxxopts::value(params.no_image_decomposition))
        ("upsample-periodization",
          "Force periodization as upsampling algorithm "
          "(default algorithm if a filter is provided). "
          "A filter is required to use this algorithm",
          cxxopts::value(params.upsample_periodization))
        ("upsample-zero-padding",
          "Force zero padding as upsampling algorithm "
          "(default algorithm if no filter is provided)",
          cxxopts::value(params.upsample_zero_padding));

    options.add_options("filter")
        ("filter",
         "Path to the filter image to apply to the source or resampled image",
         cxxopts::value(params.filter_path))
        ("filter-normalize",
         "Normalize filter coefficients "
         "(default is no normalization)",
         cxxopts::value(params.filter_normalize))
        ("zero-pad-real-edges",
         "Force zero padding strategy on real input edges "
         "(default: mirror padding)",
         cxxopts::value(params.zero_pad_real_edges))
        ("hot-point-x",
         "Hot point x coordinate "
         "(considered centered if no value is provided)",
         cxxopts::value(params.hot_point_x))
        ("hot-point-y",
         "Hot point y coordinate "
         "(considered centered if no value is provided)",
         cxxopts::value(params.hot_point_y));

    options.add_options("streaming")
        ("stream", "Enable stream mode",
         cxxopts::value(params.stream_mode))
        ("block-width", "Initial width of a stream block",
         cxxopts::value(params.stream_block_width)->default_value("256"))
        ("block-height", "Initial height of a stream block",
         cxxopts::value(params.stream_block_height)->default_value("256"))
        ("no-block-resizing",
         "Disable block resizing optimization",
         cxxopts::value(params.stream_no_block_resizing))
        ("parallel-workers", stream_parallel_workers_desc.str(),
         cxxopts::value(params.stream_parallel_workers)
            ->default_value("1")
            ->implicit_value("1"));

    options.add_options("positional arguments")
        ("i,input", "Input image", cxxopts::value(params.input_image_path))
        ("o,output", "Output image", cxxopts::value(params.output_image_path));
    // clang-format on

    options.parse_positional({"input", "output"});

    params.help_message =
          options.help({"", "resampling", "filter", "streaming"});

    try {
        auto result = options.parse(argc, argv);
        params.parsed = true;
        if (result.count("help")) {
            params.help_requested = true;
            return params;
        }
    } catch (const std::exception& e) {
        std::cerr << "sirius: cannot parse command line: " << e.what()
                  << std::endl;
        return params;
    }

    params.parsed = true;
    return params;
}
