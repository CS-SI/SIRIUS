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

#include "sirius/filter.h"

#include <cstring>

#include "sirius/exception.h"
#include "sirius/frequency_resampler_factory.h"
#include "sirius/i_frequency_resampler.h"

#include "sirius/fftw/fftw.h"
#include "sirius/fftw/wrapper.h"

#include "sirius/utils/gsl.h"
#include "sirius/utils/numeric.h"

namespace sirius {

Image ZoomFilterImageToInputResolution(const Image& filter_image,
                                       const ZoomRatio& zoom_ratio);

void NormalizeFilterImage(Image& filter_image, int oversampling);

Image FrequencyShift(const Image& filter_image, const Point& hot_point,
                     float shift_col, float shift_row);

Image CenterFilterImage(const Image& filter_image, const Point& hot_point);

Filter Filter::Create(Image filter_image, const ZoomRatio& zoom_ratio,
                      const Point& hot_point, PaddingType padding_type,
                      bool normalize) {
    if (hot_point.x < -1 || hot_point.x >= filter_image.size.col ||
        hot_point.y < -1 || hot_point.y >= filter_image.size.row) {
        LOG("filter", error, "Invalid hot point with coordinates {}, {}",
            hot_point.x, hot_point.y);
        throw sirius::Exception("Invalid hot point");
    }

    if (normalize) {
        NormalizeFilterImage(filter_image, zoom_ratio.input_resolution());
    }

    LOG("filter", info, "input filter size: {}x{}", filter_image.size.row,
        filter_image.size.col);

    if (zoom_ratio.ratio() <= 1) {
        return CreateZoomOutFilter(std::move(filter_image), zoom_ratio,
                                   padding_type, hot_point);
    } else if (!zoom_ratio.IsRealZoom()) {
        return CreateZoomInFilter(std::move(filter_image), zoom_ratio,
                                  padding_type, hot_point);
    } else {
        return CreateRealZoomFilter(std::move(filter_image), zoom_ratio,
                                    padding_type, hot_point);
    }
}

Filter::Filter(Image&& filter_image, const Size& padding_size,
               const ZoomRatio& zoom_ratio, PaddingType padding_type,
               const Point& hot_point)
    : filter_(std::move(filter_image)),
      padding_size_(padding_size),
      zoom_ratio_(zoom_ratio),
      padding_type_(padding_type),
      hot_point_(hot_point),
      filter_fft_cache_(std::make_unique<FilterFFTCache>()) {
    LOG("filter", info, "filter size: {}x{}", filter_.size.row,
        filter_.size.col);
    LOG("filter", info, "filter padding: {}x{}", padding_size_.row,
        padding_size_.col);
}

fftw::ComplexUPtr Filter::Process(const Size& image_size,
                                  fftw::ComplexUPtr image_fft) const {
    if (!IsLoaded()) {
        return image_fft;
    }

    if (image_size.row < filter_.size.row ||
        image_size.col < filter_.size.col) {
        LOG("filter", error,
            "filter {}x{} is too large to be applied on the image {}x{}",
            filter_.size.row, filter_.size.col, image_size.row, image_size.col);
        throw sirius::Exception(
              "filter is too large to be applied on the image");
    }

    int filter_fft_row = image_size.row;
    int filter_fft_col = (image_size.col / 2 + 1);
    int filter_fft_count = filter_fft_row * filter_fft_col;

#ifdef SIRIUS_ENABLE_CACHE_OPTIMIZATION
    // cache version
    auto filter_fft = filter_fft_cache_->Get(image_size);
    if (filter_fft == nullptr) {
        // create filter fft and cache it
        LOG("filter", trace, "cache filter fft for image {}x{}", image_size.row,
            image_size.col);
        fftw::ComplexUPtr uptr_filter_fft = CreateFilterFFT(image_size);
        filter_fft = {std::move(uptr_filter_fft)};
        filter_fft_cache_->Insert(image_size, filter_fft);
    }
#else
    // no cache version
    fftw::ComplexSPtr filter_fft{std::move(CreateFilterFFT(image_size))};
#endif  // SIRIUS_ENABLE_CACHE_OPTIMIZATION

    auto image_fft_span = utils::MakeSmartPtrArraySpan(image_fft, image_size);
    auto filter_fft_span = utils::MakeSmartPtrArraySpan(filter_fft, image_size);

    // apply filter on image (filter x image)
    //(a+ib)*(a'+ib') = (aa'-bb')+i(ab'+ba')
    LOG("filter", trace, "apply filter {}x{} on image FFT {}x{}",
        filter_.size.row, filter_.size.col, image_size.row, image_size.col);
    for (int fft_index = 0; fft_index < filter_fft_count; ++fft_index) {
        image_fft_span[fft_index][0] =
              filter_fft_span[fft_index][0] * image_fft_span[fft_index][0] -
              filter_fft_span[fft_index][1] * image_fft_span[fft_index][1];
        image_fft_span[fft_index][1] =
              filter_fft_span[fft_index][0] * image_fft_span[fft_index][1] +
              filter_fft_span[fft_index][1] * image_fft_span[fft_index][0];
    }

    return image_fft;
}

fftw::ComplexUPtr Filter::CreateFilterFFT(const Size& image_size) const {
    LOG("filter", trace, "pad filter image");
    // pad filter, remains in the center
    // TODO: use Image.CreateZeroPaddedImage?
    std::vector<double> filter_values(image_size.CellCount(), 0);
    int lower_row = image_size.row / 2 - (filter_.size.row - 1) / 2;
    int upper_row = image_size.row / 2 + (filter_.size.row - 1) / 2;
    int lower_col = image_size.col / 2 - (filter_.size.col - 1) / 2;
    int upper_col = image_size.col / 2 + (filter_.size.col - 1) / 2;

    auto filter_values_span = gsl::as_multi_span(filter_values);
    for (int row = lower_row; row <= upper_row; ++row) {
        for (int col = lower_col; col <= upper_col; ++col) {
            filter_values_span[row * image_size.col + col] =
                  filter_.Get(row - lower_row, col - lower_col);
        }
    }

    // filter must be unshifted in order to have zero frequency in top left
    // corner. fft expects signal to be between 0 and Fe, not -Fe/2, Fe/2
    LOG("filter", trace, "shift filter image");
    auto shifted_values = fftw::CreateReal({image_size.row, image_size.col});
    utils::IFFTShift2D(filter_values.data(), image_size, shifted_values.get());

    LOG("filter", trace, "compute filter FFT");
    return fftw::FFT(shifted_values.get(), image_size);
}

Filter Filter::CreateZoomOutFilter(Image filter_image,
                                   const ZoomRatio& zoom_ratio,
                                   PaddingType padding_type,
                                   const Point& hot_point) {
    LOG("filter", info, "filter: downsampling");
    if (hot_point.x != -1 && hot_point.y != -1) {
        filter_image = CenterFilterImage(filter_image, hot_point);
    }

    int padding_row = (filter_image.size.row % 2 == 0)
                            ? (filter_image.size.row / 2)
                            : (filter_image.size.row - 1) / 2;
    int padding_col = (filter_image.size.col % 2 == 0)
                            ? (filter_image.size.col / 2)
                            : (filter_image.size.col - 1) / 2;

    return {std::move(filter_image),
            {padding_row, padding_col},
            zoom_ratio,
            padding_type,
            hot_point};
}

Filter Filter::CreateZoomInFilter(Image filter_image,
                                  const ZoomRatio& zoom_ratio,
                                  PaddingType padding_type,
                                  const Point& hot_point) {
    LOG("filter", info, "filter: upsampling");
    if (hot_point.x != -1 && hot_point.y != -1) {
        filter_image = CenterFilterImage(filter_image, hot_point);
    }

    int padding_row = static_cast<int>(
          ((filter_image.size.row - 1) / 2) *
          (zoom_ratio.output_resolution() /
           static_cast<double>(zoom_ratio.input_resolution())));
    if (filter_image.size.row % 2 == 0) {
        padding_row = static_cast<int>(
              (filter_image.size.row / 2.) *
              (zoom_ratio.output_resolution() /
               static_cast<double>(zoom_ratio.input_resolution())));
    }
    int padding_col = static_cast<int>(
          (filter_image.size.col - 1) / 2. *
          (zoom_ratio.output_resolution() /
           static_cast<double>(zoom_ratio.input_resolution())));
    if (filter_image.size.col % 2 == 0) {
        padding_col = static_cast<int>(
              filter_image.size.col / 2. *
              (zoom_ratio.output_resolution() /
               static_cast<double>(zoom_ratio.input_resolution())));
    }

    return {std::move(filter_image),
            {padding_row, padding_col},
            zoom_ratio,
            padding_type,
            hot_point};
}

Filter Filter::CreateRealZoomFilter(Image filter_image,
                                    const ZoomRatio& zoom_ratio,
                                    PaddingType padding_type,
                                    const Point& hot_point) {
    LOG("filter", info,
        "filter: float upsampling factor (upsample filter to input "
        "resolution)");
    if (hot_point.x != -1 && hot_point.y != 1) {
        filter_image = CenterFilterImage(filter_image, hot_point);
    }

    // zoom filter image to input resolution
    filter_image = ZoomFilterImageToInputResolution(filter_image, zoom_ratio);

    // multiply by 1/input_res because zoomed_filter is now at res
    // zoom_r.input_res and its output_res is 1
    int padding_row = static_cast<int>(((filter_image.size.row - 1) / 2) *
                                       (1. / zoom_ratio.input_resolution()));
    if (filter_image.size.row % 2 == 0) {
        padding_row = static_cast<int>((filter_image.size.row / 2.) *
                                       (1. / zoom_ratio.input_resolution()));
    }
    int padding_col = static_cast<int>((filter_image.size.col - 1) / 2. *
                                       (1. / zoom_ratio.input_resolution()));
    if (filter_image.size.col % 2 == 0) {
        padding_col = static_cast<int>(filter_image.size.col / 2. *
                                       (1. / zoom_ratio.input_resolution()));
    }
    return {std::move(filter_image),
            {padding_row, padding_col},
            zoom_ratio,
            padding_type,
            hot_point};
}

Image ZoomFilterImageToInputResolution(const Image& filter_image,
                                       const ZoomRatio& zoom_ratio) {
    LOG("filter", trace, "zoom filter to input resolution");
    ImageDecompositionPolicies image_decomposition_policy =
          ImageDecompositionPolicies::kRegular;
    FrequencyZoomStrategies zoom_strategy =
          FrequencyZoomStrategies::kZeroPadding;
    auto frequency_resampler = FrequencyResamplerFactory::Create(
          image_decomposition_policy, zoom_strategy);

    auto new_zoom_ratio = ZoomRatio::Create(zoom_ratio.output_resolution(), 1);

    Image shifted_filter(filter_image.size);
    utils::IFFTShift2D(filter_image.data.data(), filter_image.size,
                       shifted_filter.data.data());

    auto zoomed_filter = frequency_resampler->Compute(
          new_zoom_ratio, shifted_filter, {0, 0, 0, 0}, {});

    Image unshifted_zoomed_filter(zoomed_filter.size);
    // centered FFTShift so filter's hot point remains centered
    utils::FFTShift2D(zoomed_filter.data.data(), zoomed_filter.size,
                      unshifted_zoomed_filter.data.data());

    NormalizeFilterImage(unshifted_zoomed_filter,
                         zoom_ratio.input_resolution());

    return unshifted_zoomed_filter;
}

void NormalizeFilterImage(Image& filter_image, int oversampling) {
    LOG("filter", trace, "normalize zoomed filter");

    // iterate over sub filters
    for (int i = 0; i < oversampling; ++i) {
        for (int j = 0; j < oversampling; ++j) {
            // calculate sum of each point for the current sub filter
            double sum = 0;
            for (int row = i; row < filter_image.size.row;
                 row += oversampling) {
                for (int col = j; col < filter_image.size.col;
                     col += oversampling) {
                    sum += filter_image.data[row * filter_image.size.col + col];
                }
            }

            // normalize each point of the current sub filter
            for (int row = i; row < filter_image.size.row;
                 row += oversampling) {
                for (int col = j; col < filter_image.size.col;
                     col += oversampling) {
                    filter_image.data[row * filter_image.size.col + col] /=
                          oversampling * oversampling * sum;
                }
            }
        }
    }
}

Image FrequencyShift(const Image& filter_image, const Point& hot_point,
                     float shift_row, float shift_col) {
    LOG("filter", info, "apply frequency shift to the filter");
    Image shifted_filter(filter_image.size);

    if (hot_point.x == -1 && hot_point.y == -1) {
        utils::IFFTShift2D(filter_image.data.data(), filter_image.size,
                           shifted_filter.data.data());
    } else {
        utils::IFFTShift2DUncentered(filter_image.data.data(),
                                     filter_image.size, hot_point,
                                     shifted_filter.data.data());
    }

    auto fft_filter =
          fftw::FFT(shifted_filter.data.data(), shifted_filter.size);
    int fft_row_count = shifted_filter.size.row;
    int fft_col_count = shifted_filter.size.col / 2 + 1;

    std::vector<double> freq_y = utils::ComputeFFTFreq(shifted_filter.size.col);
    std::vector<double> freq_x =
          utils::ComputeFFTFreq(shifted_filter.size.row, false);

    auto exp_cplx_y = fftw::CreateComplex({1, fft_col_count});
    auto exp_cplx_x = fftw::CreateComplex({fft_row_count, 1});

    for (int j = 0; j < fft_col_count; ++j) {
        exp_cplx_y.get()[j][0] = cos(-2 * M_PI * shift_row * freq_y[j]);
        exp_cplx_y.get()[j][1] = sin(-2 * M_PI * shift_row * freq_y[j]);
    }

    for (int i = 0; i < fft_row_count; ++i) {
        exp_cplx_x.get()[i][0] = cos(-2 * M_PI * shift_col * freq_x[i]);
        exp_cplx_x.get()[i][1] = sin(-2 * M_PI * shift_col * freq_x[i]);
    }

    for (int i = 0; i < fft_row_count; ++i) {
        for (int j = 0; j < fft_col_count; ++j) {
            int idx = i * fft_col_count + j;
            auto tmp_real = fft_filter.get()[idx][0];
            auto tmp_im = fft_filter.get()[idx][1];
            fft_filter.get()[idx][0] =
                  fft_filter.get()[idx][0] * exp_cplx_x[i][0] *
                        exp_cplx_y[j][0] -
                  fft_filter.get()[idx][1] * exp_cplx_x[i][1] *
                        exp_cplx_y[j][0] -
                  fft_filter.get()[idx][1] * exp_cplx_x[i][0] *
                        exp_cplx_y[j][1] -
                  fft_filter.get()[idx][0] * exp_cplx_x[i][1] *
                        exp_cplx_y[j][1];

            fft_filter.get()[idx][1] =
                  tmp_real * exp_cplx_x[i][0] * exp_cplx_y[j][1] -
                  tmp_im * exp_cplx_x[i][1] * exp_cplx_y[j][1] +
                  tmp_im * exp_cplx_x[i][0] * exp_cplx_y[j][0] +
                  tmp_real * exp_cplx_x[i][1] * exp_cplx_y[j][0];
        }
    }

    shifted_filter = fftw::IFFT(shifted_filter.size, std::move(fft_filter));
    Image unshifted_filter(shifted_filter.size);

    if (hot_point.x == -1 && hot_point.y == -1) {
        utils::FFTShift2D(shifted_filter.data.data(), unshifted_filter.size,
                          unshifted_filter.data.data());
    } else {
        utils::FFTShift2DUncentered(shifted_filter.data.data(),
                                    unshifted_filter.size, hot_point,
                                    unshifted_filter.data.data());
    }

    return unshifted_filter;
}

Image CenterFilterImage(const Image& filter_image, const Point& hot_point) {
    LOG("filter", trace, "center filter image");

    auto shifted_values =
          fftw::CreateReal({filter_image.size.row, filter_image.size.col});
    utils::IFFTShift2DUncentered(filter_image.data.data(), filter_image.size,
                                 hot_point, shifted_values.get());
    Image centered_filter(filter_image.size);
    // centered FFTShift so filter's hot point remains centered
    utils::FFTShift2D(shifted_values.get(), filter_image.size,
                      centered_filter.data.data());

    return centered_filter;
}

}  // namespace sirius
