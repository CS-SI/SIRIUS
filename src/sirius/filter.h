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

/**
 * \brief Data class that contains Filter metadata
 */
struct FilterMetadata {
    Size size;
    Size margin_size{0, 0};
    PaddingType padding_type{PaddingType::kMirrorPadding};
};

constexpr Point filter_default_hot_point{-1, -1};

/**
 * \brief Frequency filter
 */
class Filter {
  private:
    static constexpr int kCacheSize = 10;
    using FilterFFTCache = utils::LRUCache<Size, fftw::ComplexSPtr, kCacheSize>;
    using FilterFFTCacheUPtr = std::unique_ptr<FilterFFTCache>;

  public:
    /**
     * \brief Filter which is adapted specifically for a particular zoom ratio
     * \param filter_image image of the filter
     * \param zoom_ratio ratio on which the filter must be applied
     * \param padding_type padding type
     * \param normalize normalize filter
     *
     * \throw sirius::Exception if the filter image cannot be loaded
     */
    static Filter Create(Image filter_image, const ZoomRatio& zoom_ratio,
                         const Point& hot_point = filter_default_hot_point,
                         PaddingType padding_type = PaddingType::kMirrorPadding,
                         bool normalize = false);

    Filter() = default;

    ~Filter() = default;

    // non copyable
    Filter(const Filter&) = delete;
    Filter& operator=(const Filter&) = delete;
    // moveable
    Filter(Filter&&) = default;
    Filter& operator=(Filter&&) = default;

    /**
     * \brief Filter is loaded and ready to be applied on an image FFT
     * \return bool
     */
    bool IsLoaded() const { return filter_.IsLoaded(); }

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

    FilterMetadata Metadata() const {
        return {filter_.size, padding_size_, padding_type_};
    }

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
     * \brief Check that the filter can be applied on the given zoom ratio
     * \param zoom_ratio
     * \return bool
     */
    bool CanBeApplied(const ZoomRatio& zoom_ratio) const {
        return (zoom_ratio_.input_resolution() ==
                zoom_ratio.input_resolution());
    }

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
    static Filter CreateZoomInFilter(Image filter_image,
                                     const ZoomRatio& zoom_ratio,
                                     PaddingType padding_type,
                                     const Point& hot_point);
    static Filter CreateZoomOutFilter(Image filter_image,
                                      const ZoomRatio& zoom_ratio,
                                      PaddingType padding_type,
                                      const Point& hot_point);
    static Filter CreateRealZoomFilter(Image filter_image,
                                       const ZoomRatio& zoom_ratio,
                                       PaddingType padding_type,
                                       const Point& hot_point);

    Filter(Image&& filter_image, const Size& padding_size,
           const ZoomRatio& zoom_ratio, PaddingType padding_type,
           const Point& hot_point);

    fftw::ComplexUPtr CreateFilterFFT(const Size& image_size) const;

  private:
    Image filter_{};
    Size padding_size_{0, 0};
    ZoomRatio zoom_ratio_{};
    PaddingType padding_type_{PaddingType::kMirrorPadding};
    Point hot_point_{filter_default_hot_point};

    FilterFFTCacheUPtr filter_fft_cache_{nullptr};
};

}  // namespace sirius

#endif  // SIRIUS_FILTER_H_
