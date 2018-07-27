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

#include <catch/catch.hpp>

#include <algorithm>
#include <future>
#include <list>
#include <mutex>
#include <vector>

#include "sirius/utils/concurrent_queue.h"
#include "sirius/utils/log.h"

TEST_CASE("concurrent queue - monothread push pop", "[sirius]") {
    sirius::utils::ConcurrentQueue<int> queue(10);
    std::error_code push_ec;
    std::error_code pop_ec;

    std::vector<int> values = {1, 2, 3, 4, 5};
    REQUIRE(queue.IsActive());

    for (auto val : values) {
        queue.Push(std::move(val), push_ec);
        REQUIRE(!push_ec);
    }
    REQUIRE(queue.Size() == values.size());

    queue.Deactivate();

    REQUIRE(!queue.IsActive());
    while (queue.Size() > 0) {
        auto val = queue.Pop(pop_ec);
        REQUIRE(!pop_ec);
        REQUIRE(std::find(values.begin(), values.end(), val) !=
                std::end(values));
    }
    queue.Push(0, push_ec);
    REQUIRE(push_ec);
    queue.Pop(pop_ec);
    REQUIRE(pop_ec);
}

TEST_CASE("concurrent queue - multithreaded push pop", "[sirius]") {
    sirius::utils::ConcurrentQueue<int> queue(5);
    static const std::size_t kValuesSize = 1000;

    using Values = std::list<int>;

    Values values_t1;
    Values values_t2;
    Values values_t3;

    for (std::size_t i = 0; i < kValuesSize; ++i) {
        values_t1.push_back(kValuesSize + i);
        values_t2.push_back(2 * kValuesSize + i);
        values_t3.push_back(3 * kValuesSize + i);
    }
    std::mutex result_values_mutex;
    Values result_values_t1 = values_t1;
    Values result_values_t2 = values_t2;
    Values result_values_t3 = values_t3;

    REQUIRE(queue.IsActive());

    auto push_task = [&queue](const Values& values) {
        std::error_code push_ec;
        for (auto val : values) {
            queue.Push(std::move(val), push_ec);
            if (push_ec) {
                LOG("tests", error, "push ec");
            }
        }
    };

    std::size_t pop_count = 0;
    auto pop_task = [&values_t1, &values_t2, &values_t3, &result_values_t1,
                     &result_values_t2, &result_values_t3, &queue, &pop_count,
                     &result_values_mutex]() {
        while (queue.CanPop()) {
            bool value_found = false;
            std::error_code pop_ec;
            auto val = queue.Pop(pop_ec);
            if (pop_ec) {
                if (!queue.Empty()) {
                    LOG("tests", error, "pop ec while queue is not empty");
                }
                return;
            }

            std::lock_guard<std::mutex> lock(result_values_mutex);
            ++pop_count;
            if (std::find(values_t1.begin(), values_t1.end(), val) !=
                std::end(values_t1)) {
                if (std::find(result_values_t1.begin(), result_values_t1.end(),
                              val) == std::end(result_values_t1)) {
                    LOG("tests", error, "Element {} already processed", val);
                    return;
                }
                result_values_t1.remove(val);
                value_found = true;
            } else if (std::find(values_t2.begin(), values_t2.end(), val) !=
                       std::end(values_t2)) {
                if (std::find(result_values_t2.begin(), result_values_t2.end(),
                              val) == std::end(result_values_t2)) {
                    LOG("tests", error, "Element {} already processed", val);
                    return;
                }
                result_values_t2.remove(val);
                value_found = true;
            } else if (std::find(values_t3.begin(), values_t3.end(), val) !=
                       std::end(values_t3)) {
                if (std::find(result_values_t3.begin(), result_values_t3.end(),
                              val) == std::end(result_values_t3)) {
                    LOG("tests", error, "Element {} already processed", val);
                    return;
                }
                result_values_t3.remove(val);
                value_found = true;
            }
            if (!value_found) {
                LOG("tests", error, "Element {} does not exist", val);
                return;
            }
        }
    };

    auto push_t1_future = std::async(
          std::launch::async, std::bind(push_task, std::cref(values_t1)));
    auto push_t2_future = std::async(
          std::launch::async, std::bind(push_task, std::cref(values_t2)));
    auto push_t3_future = std::async(
          std::launch::async, std::bind(push_task, std::cref(values_t3)));

    auto pop_t1_future = std::async(std::launch::async, pop_task);
    auto pop_t2_future = std::async(std::launch::async, pop_task);

    push_t1_future.get();
    push_t2_future.get();
    push_t3_future.get();
    // if the queue remains active, pop operations will hang forever
    queue.Deactivate();
    pop_t1_future.get();
    pop_t2_future.get();

    REQUIRE(pop_count ==
            (values_t1.size() + values_t2.size() + values_t3.size()));
    REQUIRE(result_values_t1.size() == 0);
    REQUIRE(result_values_t2.size() == 0);
    REQUIRE(result_values_t3.size() == 0);
}
