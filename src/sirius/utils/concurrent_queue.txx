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

#ifndef SIRIUS_UTILS_CONCURRENT_QUEUE_TXX_
#define SIRIUS_UTILS_CONCURRENT_QUEUE_TXX_

#include "sirius/utils/concurrent_queue_error_code.h"

namespace sirius {
namespace utils {

template <typename T>
ConcurrentQueue<T>::ConcurrentQueue(std::size_t max_queue_size)
    : is_active_(true), max_queue_size_(max_queue_size) {}

template <typename T>
void ConcurrentQueue<T>::Push(T&& element, std::error_code& ec) {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    while (queue_.size() > max_queue_size_ && is_active_) {
        push_cond_.wait(lock);
    }

    if (!is_active_) {
        // drop element
        lock.unlock();
        ec = make_error_code(ConcurrentQueueErrorCode::kQueueIsNotActive);
        return;
    }

    queue_.emplace(std::move(element));
    lock.unlock();
    ec = make_error_code(ConcurrentQueueErrorCode::kSuccess);
    pop_cond_.notify_one();
}

template <typename T>
T ConcurrentQueue<T>::Pop(std::error_code& ec) {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    while (queue_.empty() && is_active_) {
        pop_cond_.wait(lock);
    }

    if (queue_.empty() && !is_active_) {
        lock.unlock();
        ec = make_error_code(ConcurrentQueueErrorCode::kQueueIsNotActive);
        return {};
    }

    auto element = std::move(queue_.front());
    queue_.pop();
    lock.unlock();

    ec = make_error_code(ConcurrentQueueErrorCode::kSuccess);

    push_cond_.notify_one();

    return element;
}

template <typename T>
std::size_t ConcurrentQueue<T>::Size() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return queue_.size();
}

template <typename T>
bool ConcurrentQueue<T>::Empty() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return queue_.empty();
}

template <typename T>
bool ConcurrentQueue<T>::CanPop() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return !queue_.empty() || (queue_.empty() && is_active_);
}

template <typename T>
void ConcurrentQueue<T>::Activate() {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        is_active_ = true;
    }
    push_cond_.notify_all();
    pop_cond_.notify_all();
}

template <typename T>
void ConcurrentQueue<T>::Deactivate() {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        is_active_ = false;
    }
    push_cond_.notify_all();
    pop_cond_.notify_all();
}

template <typename T>
void ConcurrentQueue<T>::DeactivateAndClear() {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        is_active_ = false;
        while (!queue_.empty()) {
            queue_.pop();
        }
    }
    push_cond_.notify_all();
    pop_cond_.notify_all();
}

template <typename T>
bool ConcurrentQueue<T>::IsActive() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return is_active_;
}

}  // namespace utils
}  // namespace sirius

#endif  // SIRIUS_UTILS_CONCURRENT_QUEUE_TXX_
