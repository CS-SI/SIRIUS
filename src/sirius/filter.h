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

#ifndef SIRIUS_FILTER_H_
#define SIRIUS_FILTER_H_

#include <map>
#include <memory>
#include <string>

#include "sirius/image.h"
#include "sirius/types.h"

#include "sirius/fftw/types.h"

#include "sirius/utils/lru_cache.h"

namespace sirius {

constexpr Point kDefaultFilterHotPoint{-1, -1};

/**
 * \brief Frequency filter
 */
class Filter {
  private:
    static constexpr int kCacheSize = 10;
    using FilterFFTCache = utils::LRUCache<Size, fftw::ComplexSPtr, kCacheSize>;
    using FilterFFTCacheUPtr = std::unique_ptr<FilterFFTCache>;

  public:
    using UPtr = std::unique_ptr<Filter>;

  public:
    /**
     * \brief Filter which is adapted specifically for a particular zoom ratio
     * \param filter_image image of the filter
     * \param zoom_ratio ratio on which the filter must be applied
     * \param hot_point hot point of the filter
     * \param padding_type padding type
     * \param normalize normalize filter
     *
     * \throw sirius::Exception if the filter image cannot be loaded
     */
    static UPtr Create(Image filter_image, const ZoomRatio& zoom_ratio,
                       const Point& hot_point = kDefaultFilterHotPoint,
                       PaddingType padding_type = PaddingType::kMirrorPadding,
                       bool normalize = false);

    /**
     * \brief Instanciate a filter
     * \param filter_image image of the filter
     * \param zoom_ratio ratio on which the filter must be applied
     * \param hot_point hot point of the filter
     * \param padding_type padding type
     * \param normalize normalize filter
     */
    Filter(Image&& filter_image, const Size& padding_size,
           const ZoomRatio& zoom_ratio, PaddingType padding_type,
           const Point& hot_point);

    ~Filter() = default;

    // non copyable
    Filter(const Filter&) = delete;
    Filter& operator=(const Filter&) = delete;
    // moveable
    Filter(Filter&&) = default;
    Filter& operator=(Filter&&) = default;

    /**
     * \brief Filter image size
     * \return Size
     */
    Size size() const { return filter_.size; }

    /**
     * \brief Padding size needed for the filter
     * \return size in row-column
     */
    Size padding_size() const { return padding_size_; }

    /**
     * \brief Get padding type
     * \return padding type
     */
    PaddingType padding_type() const { return padding_type_; }

    /**
     * \brief Zero padding needed for the filter
     * \return Padding
     */
    Padding padding() const {
        return {padding_size_.row, padding_size_.row, padding_size_.col,
                padding_size_.col, padding_type_};
    }

    const Point& hot_point() const { return hot_point_; }

    /**
     * \brief Apply the filter on the image_fft
     *
     * \remark This method is thread safe
     *
     * \param image_size size of the image of the fft
     * \param image_fft image fft computed by FFTW
     * \return the filtered fft
     *
     * \throw sirius::Exception if the filter cannot be applied on the image FFT
     */
    fftw::ComplexUPtr Process(const Size& image_size,
                              fftw::ComplexUPtr image_fft) const;

  private:
    static UPtr CreateZoomInFilter(Image filter_image,
                                   const ZoomRatio& zoom_ratio,
                                   PaddingType padding_type,
                                   const Point& hot_point);
    static UPtr CreateZoomOutFilter(Image filter_image,
                                    const ZoomRatio& zoom_ratio,
                                    PaddingType padding_type,
                                    const Point& hot_point);
    static UPtr CreateRealZoomFilter(Image filter_image,
                                     const ZoomRatio& zoom_ratio,
                                     PaddingType padding_type,
                                     const Point& hot_point);

    fftw::ComplexUPtr CreateFilterFFT(const Size& image_size) const;

  private:
    Image filter_{};
    Size padding_size_{0, 0};
    ZoomRatio zoom_ratio_{};
    PaddingType padding_type_ = PaddingType::kMirrorPadding;
    Point hot_point_ = kDefaultFilterHotPoint;

    FilterFFTCacheUPtr filter_fft_cache_{nullptr};
};

}  // namespace sirius

#endif  // SIRIUS_FILTER_H_
