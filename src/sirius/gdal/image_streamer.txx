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

#ifndef SIRIUS_GDAL_IMAGE_STREAMER_TXX_
#define SIRIUS_GDAL_IMAGE_STREAMER_TXX_

#include "sirius/gdal/image_streamer.h"

namespace sirius {
namespace gdal {

template <typename Transformer, typename InputStream, typename OutputStream>
ImageStreamer<Transformer, InputStream, OutputStream>::ImageStreamer(
      const std::string& input_path, const std::string& output_path,
      const Size& block_size, bool allow_block_resizing,
      const typename Transformer::Parameters& transformer_parameters,
      unsigned int max_parallel_workers)
    : max_parallel_workers_(std::min(max_parallel_workers,
                                     std::thread::hardware_concurrency())),
      input_stream_(input_path, block_size, allow_block_resizing,
                    transformer_parameters),
      output_stream_(input_path, output_path, transformer_parameters) {}

template <typename Transformer, typename InputStream, typename OutputStream>
void ImageStreamer<Transformer, InputStream, OutputStream>::Stream(
      const Transformer& transformer,
      const typename Transformer::Parameters& parameters) {
    if (max_parallel_workers_ == 1) {
        RunMonothreadStream(transformer, parameters);
    } else {
        RunMultithreadStream(transformer, parameters);
    }
}

template <typename Transformer, typename InputStream, typename OutputStream>
void ImageStreamer<Transformer, InputStream, OutputStream>::RunMonothreadStream(
      const Transformer& transformer,
      const typename Transformer::Parameters& parameters) {
    LOG("image_streamer", info, "start monothreaded streaming");
    while (!input_stream_.IsEnded()) {
        std::error_code read_ec;
        auto block = input_stream_.Read(read_ec);
        if (read_ec) {
            LOG("image_streamer", error, "error while reading block: {}",
                read_ec.message());
            break;
        }

        block.buffer = transformer.Compute(block.buffer,
                                           block.padding, parameters);
        std::error_code write_ec;
        output_stream_.Write(std::move(block), write_ec);
        if (write_ec) {
            LOG("image_streamer", error, "error while writing block: {}",
                write_ec.message());
            break;
        }
    }
    LOG("image_streamer", info, "end monothreaded streaming");
}

template <typename Transformer, typename InputStream, typename OutputStream>
void ImageStreamer<Transformer, InputStream, OutputStream>::
      RunMultithreadStream(const Transformer& transformer,
                           const typename Transformer::Parameters& parameters) {
    LOG("image_streamer", info, "start multithreaded streaming");

    // use block queues
    utils::ConcurrentQueue<StreamBlock> input_queue(max_parallel_workers_);
    utils::ConcurrentQueue<StreamBlock> output_queue(max_parallel_workers_);

    auto input_stream_task = [this, &input_queue]() {
        LOG("image_streamer", info, "start reading blocks");
        while (!input_stream_.IsEnded() && input_queue.IsActive()) {
            std::error_code read_ec;
            auto block = input_stream_.Read(read_ec);
            if (read_ec) {
                LOG("image_streamer", error, "error while reading block: {}",
                    read_ec.message());
                break;
            }

            std::error_code push_input_ec;
            input_queue.Push(std::move(block), push_input_ec);
            if (push_input_ec) {
                LOG("image_streamer", error,
                    "cannot push input block into input queue: {}",
                    push_input_ec.message());
                break;
            }
        }
        input_queue.Deactivate();
        LOG("image_streamer", info, "end reading blocks");
    };

    auto worker_task = [&input_queue, &output_queue, &transformer,
                        &parameters]() {
        try {
            while (input_queue.CanPop()) {
                std::error_code pop_input_ec;
                auto block = input_queue.Pop(pop_input_ec);
                if (pop_input_ec) {
                    // no more block to process
                    LOG("image_streamer", debug,
                        "cannot pop input block from input queue: {}",
                        pop_input_ec.message());
                    break;
                }

                block.buffer = transformer.Compute(
                      block.buffer, block.padding, parameters);

                std::error_code push_output_ec;
                output_queue.Push(std::move(block), push_output_ec);
                if (push_output_ec) {
                    LOG("image_streamer", error,
                        "cannot push computed block into output queue: {}",
                        push_output_ec.message());
                    break;
                }
            }
        } catch (const std::exception& e) {
            LOG("image_streamer", error, "exception while processing block: {}",
                e.what());
            input_queue.Deactivate();
            output_queue.Deactivate();
        }
    };

    auto output_stream_task = [this, &output_queue]() {
        LOG("image_streamer", info, "start writing blocks");
        while (output_queue.CanPop()) {
            std::error_code pop_output_ec;
            std::error_code write_ec;
            auto block = output_queue.Pop(pop_output_ec);
            if (pop_output_ec) {
                // no more block to process
                break;
            }
            output_stream_.Write(std::move(block), write_ec);
            if (write_ec) {
                LOG("image_streamer", error, "error while writing block: {}",
                    write_ec.message());
                output_queue.DeactivateAndClear();
            }
        }
        output_queue.Deactivate();
        LOG("image_streamer", info, "end writing blocks");
    };

    auto output_task_future =
          std::async(std::launch::async, output_stream_task);
    auto input_task_future = std::async(std::launch::async, input_stream_task);

    LOG("image_streamer", info, "start transform processing with {} workers",
        max_parallel_workers_);
    using WorkerTasks = std::vector<std::future<void>>;
    WorkerTasks worker_task_futures;
    for (unsigned int i = 0; i < max_parallel_workers_; ++i) {
        worker_task_futures.push_back(
              std::async(std::launch::async, worker_task));
    }
    for (auto& worker_task_future : worker_task_futures) {
        try {
            // wait end of task or error
            worker_task_future.get();
        } catch (const std::exception& e) {
            LOG("image_streamer", error, "exception on worker task: {}",
                e.what());
        }
    }
    LOG("image_streamer", info, "end transform processing");
    output_queue.Deactivate();
    output_task_future.get();
    input_task_future.get();
    LOG("image_streamer", info, "end multithreaded streaming");
}

}  // namespace gdal
}  // namespace sirius

#endif  // SIRIUS_GDAL_IMAGE_STREAMER_TXX_