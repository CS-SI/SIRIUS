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

#include "sirius/utils/concurrent_queue_error_code.h"

namespace sirius {
namespace utils {

class ConcurrentQueueErrorCategory : public std::error_category {
    const char* name() const noexcept override;
    std::string message(int ev) const override;
};

const char* ConcurrentQueueErrorCategory::name() const noexcept {
    return "concurrent_queue";
}

std::string ConcurrentQueueErrorCategory::message(int ev) const {
    switch (static_cast<ConcurrentQueueErrorCode>(ev)) {
        case ConcurrentQueueErrorCode::kSuccess:
            return "success";
        case ConcurrentQueueErrorCode::kQueueIsNotActive:
            return "queue is not active";
        default:
            return "unknown error";
    }
}

ConcurrentQueueErrorCategory& GetConcurrentQueueErrorCategory() {
    static ConcurrentQueueErrorCategory category;
    return category;
}

std::error_code make_error_code(ConcurrentQueueErrorCode errc) {
    return {static_cast<int>(errc), GetConcurrentQueueErrorCategory()};
}

}  // namespace utils
}  // namespace sirius
