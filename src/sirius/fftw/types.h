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

#ifndef SIRIUS_FFTW_TYPES_H_
#define SIRIUS_FFTW_TYPES_H_

#include <memory>

#include <fftw3.h>

#include "sirius/utils/log.h"

namespace sirius {
namespace fftw {

namespace detail {

/**
 * \brief Deleter of fftw_complex array for smart pointer
 */
struct ComplexDeleter {
    void operator()(::fftw_complex* complex) {
        if (complex == nullptr) {
            return;
        }
        ::fftw_free(complex);
    }
};

/**
 * \brief Deleter of fftw real array for smart pointer
 */
struct RealDeleter {
    void operator()(double* real) { ::fftw_free(real); }
};

}  // namespace detail

using ComplexUPtr = std::unique_ptr<::fftw_complex[], detail::ComplexDeleter>;

//_GLIBCXX_RELEASE is only defined for stdlibc++ >= 7
#if (!defined(_GLIBCXX_RELEASE) && !defined(_MSC_VER) && __cplusplus <= 201402L) || \
      (defined(_MSC_VER) && _MSC_VER < 1900)

// C++14: no shared_ptr array syntax, classic definition
using ComplexSPtr = std::shared_ptr<::fftw_complex>;

#else

// C++17: std::shared_ptr array syntax

// compiling with stdlibc++ > 7 or VS2017 breaks the build if C++14 syntax is used
// definition of std::shared_ptr<T>::element_type has changed:
// <7: typedef _Tp element_type;
// >=7: using element_type = typename remove_extent<_Tp>::type;
// With stdlibc <7:
//   std::shared_ptr<double[2]>::element_type <=> double[2]
// With stdlibc++ >=7 or VS2017:
//   std::shared_ptr<double[2]>::element_type <=> double
//   std::shared_ptr<double[][2]>::element_type <=> double[2]

using ComplexSPtr = std::shared_ptr<::fftw_complex[]>;

#endif  // (!defined(_GLIBCXX_RELEASE) && !defined(_MSC_VER) && __cplusplus <= 201402L) ||
        //   (defined(_MSC_VER) && _MSC_VER < 1900)

using RealUPtr = std::unique_ptr<double[], detail::RealDeleter>;

}  // namespace fftw
}  // namespace sirius

#endif  // SIRIUS_FFTW_TYPES_H_
