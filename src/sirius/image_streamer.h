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

#ifndef SIRIUS_IMAGE_STREAMER_H_
#define SIRIUS_IMAGE_STREAMER_H_

#include "sirius/filter.h"
#include "sirius/i_frequency_resampler.h"

#include "sirius/gdal/input_stream.h"
#include "sirius/gdal/resampled_output_stream.h"
#include "sirius/gdal/wrapper.h"

namespace sirius {

/**
 * \brief Image streamer with monothread or multithread strategies
 */
class ImageStreamer {
  public:
    /**
     * \brief Instanciate an image streamer which will stream input image, apply
     *   a resampling transformation and write the result into the output image
     * \param input_path input image path
     * \param output_path output image path
     * \param block_size stream block size
     * \param zoom_ratio zoom ratio
     * \param filter_metadata filter metadata
     * \param padding_type filter padding type
     * \param max_parallel_workers max parallel workers to compute the zoom on
     *        stream blocks
     */
    ImageStreamer(const std::string& input_path, const std::string& output_path,
                  const Size& block_size, const ZoomRatio& zoom_ratio,
                  const FilterMetadata& filter_metadata,
                  unsigned int max_parallel_workers);

    /**
     * \brief Stream the input image, compute the resampling and stream
     *   output data
     * \param frequency_resampler requested frequency resampler to apply on
     *   stream block
     * \param filter filter to apply on the stream block
     */
    void Stream(const IFrequencyResampler& frequency_resampler,
                const Filter& filter);

  private:
    /**
     * \brief Stream image in monothreading mode
     *
     * Read a block, compute the resample and write the output in the output
     *   file
     *
     * \param frequency_resampler frequency zoom to apply on stream block
     * \param filter filter to apply on stream block
     */
    void RunMonothreadStream(const IFrequencyResampler& frequency_resampler,
                             const Filter& filter);

    /**
     * \brief Stream image in multithreading mode
     *
     * One thread will generate input blocks and feed an input queue
     * One thread will consume resampled blocks from an output queue and write
     *   them in the output file
     * max_parallel_tasks threads will consume input blocks from an input queue,
     *   compute the resampled blocks and feed an output queue
     *
     * \param frequency_resampler frequency zoom to apply on stream block
     * \param filter filter to apply on stream block
     */
    void RunMultithreadStream(const IFrequencyResampler& frequency_resampler,
                              const Filter& filter);

  private:
    unsigned int max_parallel_workers_;
    Size block_size_;
    ZoomRatio zoom_ratio_;
    gdal::InputStream input_stream_;
    gdal::ResampledOutputStream output_stream_;
};

}  // namespace sirius

#endif  // SIRIUS_IMAGE_STREAMER_H_
