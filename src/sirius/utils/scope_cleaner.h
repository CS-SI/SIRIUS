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

#ifndef SIRIUS_UTILS_SCOPE_CLEANER_H_
#define SIRIUS_UTILS_SCOPE_CLEANER_H_

namespace sirius {
namespace utils {

/**
 * \brief Executes a callback at the end of the scope
 */
template <typename Cleaner>
class ScopeCleaner {
  public:
    ScopeCleaner(Cleaner cleaner) : cleaner_(cleaner) {}

    ~ScopeCleaner() {
        if (is_clean_aborted) {
            return;
        }
        cleaner_();
    }

    /**
     * \brief Abort the callback
     */
    void Abort() { is_clean_aborted = true; }

  private:
    bool is_clean_aborted{false};
    Cleaner cleaner_;
};

/**
 * \brief ScopeCleaner generator
 */
template <typename Cleaner>
ScopeCleaner<Cleaner> MakeScopeCleaner(Cleaner cleaner) {
    return {cleaner};
}

}  // namespace utils
}  // namespace sirius

#endif  // SIRIUS_UTILS_SCOPE_CLEANER_H_
