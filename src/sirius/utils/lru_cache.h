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

#ifndef SIRIUS_UTILS_LRU_CACHE_H_
#define SIRIUS_UTILS_LRU_CACHE_H_

#include <list>
#include <map>
#include <mutex>

#include "sirius/utils/log.h"

namespace sirius {
namespace utils {

/**
 * \brief LRU cache
 */
template <typename Key, typename Value, std::size_t CacheSize = 5>
class LRUCache {
  public:
    LRUCache() = default;
    ~LRUCache() = default;

    // non copyable
    LRUCache(const LRUCache&) = delete;
    LRUCache& operator=(const LRUCache&) = delete;
    // non moveable
    LRUCache(LRUCache&&) = delete;
    LRUCache& operator=(LRUCache&&) = delete;

    /**
     * \brief Get a cache element
     *
     * The element fetched will be the new head of the cache
     *
     * \param key key of the requested element
     * \return cache element if exists or default constructed element if not
     */
    Value Get(const Key& key) {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        if (elements_.count(key) > 0) {
            ordered_keys_.remove(key);
            ordered_keys_.push_front(key);
            return elements_[key];
        }
        return {};
    }

    /**
     * \brief Insert an element in the cache
     *
     * If the cache size reaches the max size, the last recently used (LRU)
     * element is removed from the cache
     *
     * \param key key of the element
     * \param element element to insert
     */
    void Insert(const Key& key, Value element) {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        if (elements_.count(key) > 0) {
            // entry already in cache, remove it
            ordered_keys_.remove(key);
            elements_.erase(key);
        }

        // insert (key, value) in cache
        ordered_keys_.push_front(key);
        elements_[key] = element;

        if (elements_.size() > CacheSize) {
            // too many entries, remove the last recently used value
            auto lru_key = ordered_keys_.back();
            ordered_keys_.pop_back();
            elements_.erase(lru_key);
        }
    }

    /**
     * \brief Remove an element from the cache
     * \param key key of the element
     */
    void Remove(const Key& key) {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        if (elements_.count(key) == 0) {
            return;
        }
        ordered_keys_.remove(key);
        elements_.erase(key);
    }

    /**
     * \brief Clear all the cache elements
     */
    void Clear() {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        ordered_keys_.clear();
        elements_.clear();
    }

    /**
     * \brief Check that a particular key is present in cache
     * \param key key of the requested element
     * \return true if the element is present in cache
     */
    bool Contains(const Key& key) const { return elements_.count(key) > 0; }

    /**
     * \brief Get the current size of the cache
     * \return size of the cache
     */
    std::size_t Size() { return elements_.size(); }

  private:
    std::mutex cache_mutex_;
    std::list<Key> ordered_keys_;
    std::map<Key, Value> elements_;
};

}  // namespace utils
}  // namespace sirius

#endif  // SIRIUS_UTILS_LRU_CACHE_H_
