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
#include "sirius/sirius.h"

#include "sirius/gdal/image_streamer.h"
#include "sirius/gdal/resampling/input_stream.h"
#include "sirius/gdal/resampling/output_stream.h"
#include "sirius/gdal/wrapper.h"

#include "sirius/utils/log.h"

struct CliOptions {
    struct Resampling {
        std::string ratio = "1:1";
        bool no_image_decomposition = false;
        bool upsample_periodization = false;
        bool upsample_zero_padding = false;
    };
    struct Filter {
        std::string path;
        bool zero_pad_real_edges = false;
        bool normalize = false;
        sirius::Point hot_point = sirius::kDefaultFilterHotPoint;
    };
    struct Stream {
        sirius::Size block_size = {256, 256};
        bool no_block_resizing = false;
        unsigned int parallel_workers = std::thread::hardware_concurrency();
    };

    // mandatory arguments
    std::string input_image_path;
    std::string output_image_path;

    // general options
    std::string verbosity_level = "info";

    Resampling resampling;
    Filter filter;
    Stream stream;

    // status
    bool parsed = false;
    bool help_requested = false;
    std::string help_message;
};

template <typename Transformer, typename InputStream, typename OutputStream>
void StreamTransformation(const std::string& input_path,
                          const std::string& output_path,
                          const CliOptions::Stream& stream_options,
                          const Transformer& transformer,
                          const typename Transformer::Parameters& parameters);

CliOptions GetCliOptions(int argc, char* argv[]);

void Resample(const std::string& input_path, const std::string& output_path,
              const CliOptions::Resampling& resampling_options,
              const CliOptions::Filter& filter_options,
              const CliOptions::Stream& stream_options);

int main(int argc, char* argv[]) {
    CliOptions options = GetCliOptions(argc, argv);
    if (options.help_requested || !options.parsed) {
        std::cerr << options.help_message;
        return options.help_requested ? 0 : 1;
    }

    if (options.input_image_path.empty() || options.output_image_path.empty()) {
        std::cerr << options.help_message << std::endl;
        std::cerr << "sirius: input and/or output arguments are missing"
                  << std::endl;
        return 1;
    }

    sirius::utils::SetVerbosityLevel(options.verbosity_level);

    LOG("sirius", info, "Sirius {} - {}", sirius::kVersion, sirius::kGitCommit);

    try {
        Resample(options.input_image_path, options.output_image_path,
                 options.resampling, options.filter, options.stream);
    } catch (const std::exception& e) {
        LOG("sirius", error, "exception while computing resampling: {}",
            e.what());
        return 1;
    }

    return 0;
}

void Resample(const std::string& input_path, const std::string& output_path,
              const CliOptions::Resampling& resampling_options,
              const CliOptions::Filter& filter_options,
              const CliOptions::Stream& stream_options) {
    sirius::ZoomRatio resampling_ratio =
          sirius::ZoomRatio::Create(resampling_options.ratio);
    LOG("sirius", info, "resampling ratio: {}:{}",
        resampling_ratio.input_resolution(),
        resampling_ratio.output_resolution());

    // filter parameters
    sirius::PaddingType padding_type = sirius::PaddingType::kMirrorPadding;
    if (filter_options.zero_pad_real_edges) {
        LOG("sirius", info, "filter: border zero padding");
        padding_type = sirius::PaddingType::kZeroPadding;
    } else {
        LOG("sirius", info, "filter: mirror padding");
    }

    if (filter_options.normalize) {
        LOG("sirius", info, "filter: normalize");
    }

    sirius::Filter::UPtr filter = nullptr;
    if (!filter_options.path.empty()) {
        LOG("sirius", info, "filter path: {}", filter_options.path);
        sirius::Point hp(filter_options.hot_point.x,
                         filter_options.hot_point.y);
        filter = sirius::Filter::Create(sirius::gdal::Load(filter_options.path),
                                        resampling_ratio, hp, padding_type,
                                        filter_options.normalize);
    }

    // resampling parameters
    sirius::image_decomposition::Policies image_decomposition_policy =
          sirius::image_decomposition::Policies::kPeriodicSmooth;
    sirius::FrequencyUpsamplingStrategies upsampling_strategy =
          sirius::FrequencyUpsamplingStrategies::kPeriodization;

    if (resampling_options.no_image_decomposition) {
        LOG("sirius", info, "image decomposition: none");
        image_decomposition_policy =
              sirius::image_decomposition::Policies::kRegular;
    } else {
        LOG("sirius", info, "image decomposition: periodic plus smooth");
    }

    if (resampling_ratio.ratio() > 1) {
        // choose the upsampling algorithm only if ratio > 1
        if (resampling_options.upsample_periodization && !filter) {
            throw sirius::Exception(
                  "filter is required with periodization upsampling");
        } else if (resampling_options.upsample_zero_padding || !filter) {
            LOG("sirius", info, "upsampling: zero padding");
            upsampling_strategy =
                  sirius::FrequencyUpsamplingStrategies::kZeroPadding;
            if (filter) {
                LOG("sirius", warn,
                    "upsampling: filter will be used with zero padding");
            }
        } else {
            LOG("sirius", info, "upsampling: periodization");
            upsampling_strategy =
                  sirius::FrequencyUpsamplingStrategies::kPeriodization;
        }
    }

    auto frequency_resampler = sirius::FrequencyResamplerFactory::Create(
          image_decomposition_policy, upsampling_strategy);

    StreamTransformation<sirius::IFrequencyResampler,
                         sirius::gdal::resampling::InputStream,
                         sirius::gdal::resampling::OutputStream>(
          input_path, output_path, stream_options, *frequency_resampler,
          {resampling_ratio, filter.get()});
}

template <typename Transformer, typename InputStream, typename OutputStream>
void StreamTransformation(const std::string& input_path,
                          const std::string& output_path,
                          const CliOptions::Stream& stream_options,
                          const Transformer& transformer,
                          const typename Transformer::Parameters& parameters) {
    unsigned int max_parallel_workers =
          std::max(std::min(stream_options.parallel_workers,
                            std::thread::hardware_concurrency()),
                   1u);

    sirius::gdal::ImageStreamer<Transformer, InputStream, OutputStream>
          streamer(input_path, output_path, stream_options.block_size,
                   !stream_options.no_block_resizing, parameters,
                   max_parallel_workers);
    streamer.Stream(transformer, parameters);
}

CliOptions GetCliOptions(int argc, char* argv[]) {
    CliOptions cli_options;
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
         cxxopts::value(cli_options.verbosity_level)->default_value("info"));

    options.add_options("resampling")
        ("r,resampling-ratio", "Resampling ratio as input:output, "
          "allowed format: I (equivalent to I:1), I:O",
         cxxopts::value(cli_options.resampling.ratio)->default_value("1:1"))
        ("no-image-decomposition",
         "Do not decompose the input image "
         "(default: periodic plus smooth image decomposition)",
         cxxopts::value(cli_options.resampling.no_image_decomposition))
        ("upsample-periodization",
          "Force periodization as upsampling algorithm "
          "(default algorithm if a filter is provided). "
          "A filter is required to use this algorithm",
          cxxopts::value(cli_options.resampling.upsample_periodization))
        ("upsample-zero-padding",
          "Force zero padding as upsampling algorithm "
          "(default algorithm if no filter is provided)",
          cxxopts::value(cli_options.resampling.upsample_zero_padding));

    options.add_options("filter")
        ("filter",
         "Path to the filter image to apply to the source or resampled image",
         cxxopts::value(cli_options.filter.path))
        ("filter-normalize",
         "Normalize filter coefficients "
         "(default is no normalization)",
         cxxopts::value(cli_options.filter.normalize))
        ("zero-pad-real-edges",
         "Force zero padding strategy on real input edges "
         "(default: mirror padding)",
         cxxopts::value(cli_options.filter.zero_pad_real_edges))
        ("hot-point-x",
         "Hot point x coordinate "
         "(considered centered if no value is provided)",
         cxxopts::value(cli_options.filter.hot_point.x))
        ("hot-point-y",
         "Hot point y coordinate "
         "(considered centered if no value is provided)",
         cxxopts::value(cli_options.filter.hot_point.y));

    options.add_options("streaming")
        ("block-width", "Initial width of a stream block",
         cxxopts::value(cli_options.stream.block_size.col)->default_value("256"))
        ("block-height", "Initial height of a stream block",
         cxxopts::value(cli_options.stream.block_size.row)->default_value("256"))
        ("no-block-resizing",
         "Disable block resizing optimization",
         cxxopts::value(cli_options.stream.no_block_resizing))
        ("parallel-workers", stream_parallel_workers_desc.str(),
         cxxopts::value(cli_options.stream.parallel_workers)
            ->default_value("1")
            ->implicit_value("1"));

    options.add_options("positional arguments")
        ("i,input", "Input image", cxxopts::value(cli_options.input_image_path))
        ("o,output", "Output image", cxxopts::value(cli_options.output_image_path));
    // clang-format on

    options.parse_positional({"input", "output"});

    cli_options.help_message =
          options.help({"", "resampling", "filter", "streaming"});

    try {
        auto result = options.parse(argc, argv);
        cli_options.parsed = true;
        if (result.count("help")) {
            cli_options.help_requested = true;
            return cli_options;
        }
    } catch (const std::exception& e) {
        std::cerr << "sirius: cannot parse command line: " << e.what()
                  << std::endl;
        return cli_options;
    }

    return cli_options;
}
