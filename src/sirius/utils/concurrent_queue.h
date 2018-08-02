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

#ifndef SIRIUS_UTILS_CONCURRENT_QUEUE_H_
#define SIRIUS_UTILS_CONCURRENT_QUEUE_H_

#include <condition_variable>
#include <mutex>
#include <queue>
#include <system_error>

namespace sirius {
namespace utils {

/**
 * \brief Concurrent queue
 *
 * ConcurrentQueue is a basic thread safe implementation of a queue.
 * ConcurrentQueue has a limited size.
 * Push and Pop methods can be used in a multi-threaded context.
 *
 * This implementation is inspired by
 *   https://juanchopanzacpp.wordpress.com/2013/02/26/concurrent-queue-c11
 *
 * \warning Push and Pop methods may block.
 */
template <typename T>
class ConcurrentQueue {
  public:
    /**
     * \brief Instanciate a concurrent queue with a maximum size
     *
     * The queue is active after instantiation
     *
     * \param max_queue_size limit the queue size
     */
    ConcurrentQueue(std::size_t max_queue_size = 10);
    ~ConcurrentQueue() = default;

    ConcurrentQueue(const ConcurrentQueue&) = delete;
    ConcurrentQueue& operator=(const ConcurrentQueue&) = delete;
    ConcurrentQueue(ConcurrentQueue&&) = delete;
    ConcurrentQueue& operator=(ConcurrentQueue&&) = delete;

    /**
     * \brief Push an element in the queue
     *
     * \param element to push to the queue
     * \param ec return status
     *
     * \warning This method may block until there is an available spot
     * \warning If the queue is not active, the element will be dropped and ec
     *          is set
     */
    void Push(T&& element, std::error_code& ec);

    /**
     * \brief Pop an element from the queue
     *
     * \param ec return status
     * \return An available element from the queue
     *
     * \warning This method may block until there is an element to pop
     * \warning If no element is available and the queue is not active,
     *          this method returns a default constructed element and ec is set
     */
    T Pop(std::error_code& ec);

    /**
     * \brief Get queue size
     *
     * \return Queue size
     */
    std::size_t Size();

    /**
     * \brief Queue is empty
     *
     * \return true if queue is empty
     */
    bool Empty();

    /**
     * \brief Elements can be popped from the queue
     *
     * \return boolean
     */
    bool CanPop();

    /**
     * \brief Activate the queue
     *
     * The queue will be able to receive new elements
     */
    void Activate();

    /**
     * \brief Deactivate the queue
     *
     * The queue will not be able to receive new elements
     */
    void Deactivate();

    /**
     * \brief Deactivate the queue and clear its content
     *
     * The queue will not be able to receive new elements and its content will
     * be erased
     */
    void DeactivateAndClear();

    /**
     * \brief Can the queue be filled with new elements
     *
     * \return true if the queue is still active
     */
    bool IsActive();

  private:
    std::mutex queue_mutex_;
    bool is_active_{true};
    std::size_t max_queue_size_{10};
    std::queue<T> queue_;
    std::condition_variable push_cond_;
    std::condition_variable pop_cond_;
};

}  // namespace utils
}  // namespace sirius

#include "sirius/utils/concurrent_queue.txx"

#endif  // SIRIUS_UTILS_CONCURRENT_QUEUE_H_
