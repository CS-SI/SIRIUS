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

#include "sirius/fftw/fftw.h"

#include <type_traits>

#include <fftw3.h>

#include "sirius/fftw/exception.h"
#include "sirius/fftw/wrapper.h"

#include "sirius/utils/log.h"

namespace sirius {
namespace fftw {

namespace detail {

void PlanDeleter::operator()(::fftw_plan plan) {
    Fftw::Instance().DestroyPlan(plan);
}

}  // namespace detail

Fftw& Fftw::Instance() {
    static Fftw instance;
    return instance;
}

PlanSPtr Fftw::GetRealToComplexPlan(const Size& size, double* in,
                                    fftw_complex* out) {
    LOG("fftw", trace, "get r2c plan {}x{}", size.row, size.col);

#ifdef SIRIUS_ENABLE_CACHE_OPTIMIZATION
    // cache version
    auto r2c_plan = r2c_plans_.Get(size);
    if (r2c_plan == nullptr) {
        LOG("fftw", trace, "cache r2c plan {}x{}", size.row, size.col);
        r2c_plan = CreateR2CPlan(size, in, out);
        r2c_plans_.Insert(size, r2c_plan);
    }
#else
    // no cache version
    auto r2c_plan = CreateR2CPlan(size, in, out);
#endif  // SIRIUS_ENABLE_CACHE_OPTIMIZATION

    return r2c_plan;
}

PlanSPtr Fftw::GetComplexToRealPlan(const Size& size, fftw_complex* in,
                                    double* out) {
    LOG("fftw", trace, "get c2r plan {}x{}", size.row, size.col);

#ifdef SIRIUS_ENABLE_CACHE_OPTIMIZATION
    // cache version
    auto c2r_plan = c2r_plans_.Get(size);
    if (c2r_plan == nullptr) {
        LOG("fftw", trace, "cache c2r plan {}x{}", size.row, size.col);
        c2r_plan = CreateC2RPlan(size, in, out);
        c2r_plans_.Insert(size, c2r_plan);
    }
#else
    // no cache version
    auto c2r_plan = CreateC2RPlan(size, in, out);
#endif  // SIRIUS_ENABLE_CACHE_OPTIMIZATION

    return c2r_plan;
}

PlanSPtr Fftw::CreateC2RPlan(const Size& size, fftw_complex* in, double* out) {
    std::lock_guard<std::mutex> lock(plan_mutex_);
    PlanSPtr c2r_plan(
          fftw_plan_dft_c2r_2d(size.row, size.col, in, out, FFTW_ESTIMATE),
          detail::PlanDeleter());
    if (c2r_plan == nullptr) {
        LOG("fftw", error, "cannot create c2r plan {}x{}", size.row, size.row);
        throw Exception(fftw::ErrorCode::kPlanCreationFailed);
    }
    return c2r_plan;
}

PlanSPtr Fftw::CreateR2CPlan(const Size& size, double* in, fftw_complex* out) {
    std::lock_guard<std::mutex> lock(plan_mutex_);
    PlanSPtr r2c_plan(
          fftw_plan_dft_r2c_2d(size.row, size.col, in, out, FFTW_ESTIMATE),
          detail::PlanDeleter());
    if (r2c_plan == nullptr) {
        LOG("fftw", error, "cannot create r2c plan {}x{}", size.row, size.row);
        throw Exception(fftw::ErrorCode::kPlanCreationFailed);
    }
    return r2c_plan;
}

void Fftw::DestroyPlan(::fftw_plan plan) {
    if (plan == nullptr) {
        return;
    }

    std::lock_guard<std::mutex> lock(plan_mutex_);
    ::fftw_destroy_plan(plan);
}

}  // namespace fftw
}  // namespace sirius
