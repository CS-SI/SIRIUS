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

#ifndef SIRIUS_UTILS_GSL_H_
#define SIRIUS_UTILS_GSL_H_

#include <gsl/gsl>

namespace sirius {
namespace utils {

/**
 * \brief Make a GSL span from a smart pointer pointing to an array of data
 * \param smart_ptr smart pointer to array to wrap as span
 * \param size size of the array
 * \return span view of the array
 */
template <typename SmartPtr>
auto MakeSmartPtrArraySpan(const SmartPtr& smart_ptr, const Size& size) {
    return gsl::span<typename SmartPtr::element_type>(smart_ptr.get(),
                                                      size.CellCount());
}

}  // namespace utils
}  // namespace sirius

#endif  // SIRIUS_UTILS_GSL_H_