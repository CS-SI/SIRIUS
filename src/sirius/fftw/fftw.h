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

#ifndef SIRIUS_FFTW_FFTW_H_
#define SIRIUS_FFTW_FFTW_H_

#include <map>
#include <memory>
#include <type_traits>

#include "sirius/fftw/types.h"
#include "sirius/image.h"
#include "sirius/types.h"

#include "sirius/utils/lru_cache.h"

namespace sirius {
namespace fftw {

namespace detail {

/**
 * \brief Deleter of fftw_plan for smart pointer
 */
struct PlanDeleter {
    void operator()(::fftw_plan plan);
};
}  // namespace detail

using PlanUPtr =
      std::unique_ptr<std::remove_pointer_t<::fftw_plan>, detail::PlanDeleter>;
using PlanSPtr = std::shared_ptr<std::remove_pointer_t<::fftw_plan>>;

/**
 * \brief fftw3 management class
 */
class Fftw {
  private:
    static constexpr int kCacheSize = 10;
    using PlanCache = utils::LRUCache<Size, PlanSPtr, kCacheSize>;

  public:
    /**
     * \brief Get Fftw singleton instance
     * \return Fftw instance
     */
    static Fftw& Instance();

    /**
     * \brief Get a r2c fftw plan of the given size
     * \param in real input array complying with the size
     * \param out complex output array complying with the size
     * \return unique ptr to the created plan
     * \throws sirius::fftw::Exception if the plan creation fails
     */
    PlanSPtr GetRealToComplexPlan(const Size& size, double* in,
                                  fftw_complex* out);
    /**
     * \brief Get a c2r fftw plan of the given size
     * \param size plan size
     * \param in complex input array complying with the size
     * \param out real output array complying with the size
     * \throws sirius::fftw::Exception if the plan creation fails
     */
    PlanSPtr GetComplexToRealPlan(const Size& size, fftw_complex* in,
                                  double* out);

  private:
    Fftw() = default;

    // not copyable
    Fftw(const Fftw&) = delete;
    Fftw operator=(const Fftw&) = delete;
    // not moveable
    Fftw(Fftw&&) = delete;
    Fftw operator=(Fftw&&) = delete;

    PlanSPtr CreateC2RPlan(const Size& size, fftw_complex* in, double* out);
    PlanSPtr CreateR2CPlan(const Size& size, double* out, fftw_complex* in);

    // allow PlanDeleter operator() to access private DestroyPlan method
    friend void detail::PlanDeleter::operator()(::fftw_plan);
    void DestroyPlan(::fftw_plan plan);

  private:
    std::mutex plan_mutex_;

    PlanCache r2c_plans_;
    PlanCache c2r_plans_;
};

}  // namespace fftw
}  // namespace sirius

#endif  // SIRIUS_FFTW_FFTW_H_
