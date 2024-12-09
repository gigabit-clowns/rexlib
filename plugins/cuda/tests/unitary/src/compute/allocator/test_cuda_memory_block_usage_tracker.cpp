/***************************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 *
 *  All comments concerning this program package may be sent to the
 *  e-mail address 'xmipp@cnb.csic.es'
 ***************************************************************************/

/**
 * @file cuda_memory_block_usage_tracker.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Tests for cuda_memory_block_usage_tracker.hpp
 * @date 2024-12-09
 * 
 */

#include <xmipp4/cuda/compute/allocator/cuda_memory_block_usage_tracker.hpp>

#include <cstddef>

#include <catch2/catch_test_macros.hpp>
#include <trompeloeil.hpp>

using namespace xmipp4;
using namespace xmipp4::compute;

TEST_CASE("adding unique queue on an cuda_memory_block_usage_tracker should succeed", "[cuda_memory_block_usage_tracker]")
{
    cuda_device_queue *queue0 = nullptr;
    auto *queue1 = reinterpret_cast<cuda_device_queue*>(std::uintptr_t(0xDEADBEEF));
    auto *queue2 = reinterpret_cast<cuda_device_queue*>(std::uintptr_t(0x12341234));

    const cuda_memory_block block(nullptr, 0UL, queue0);
    cuda_memory_block_usage_tracker tracker;
    tracker.add_queue(block, *queue1);
    tracker.add_queue(block, *queue2);

    // Expect the result to be ordered address-wise.
    const auto queues = tracker.get_queues();
    REQUIRE(queues.size() == 2);
    REQUIRE(queues[0] == queue2);
    REQUIRE(queues[1] == queue1);
}

TEST_CASE("adding block's queue to a cuda_memory_block_usage_tracker should not have an effect", "[cuda_memory_block_usage_tracker]")
{
    auto *queue1 = reinterpret_cast<cuda_device_queue*>(std::uintptr_t(0xDEADBEEF));

    const cuda_memory_block block(nullptr, 0UL, queue1);
    cuda_memory_block_usage_tracker tracker;
    tracker.add_queue(block, *queue1);

    const auto queues = tracker.get_queues();
    REQUIRE(queues.size() == 0);
}

TEST_CASE("adding the same queue for a second time to a cuda_memory_block_usage_tracker not have an effect", "[cuda_memory_block_usage_tracker]")
{
    cuda_device_queue *queue0 = nullptr;
    auto *queue1 = reinterpret_cast<cuda_device_queue*>(std::uintptr_t(0xDEADBEEF));

    const cuda_memory_block block(nullptr, 0UL, queue0);
    cuda_memory_block_usage_tracker tracker;
    tracker.add_queue(block, *queue1);
    tracker.add_queue(block, *queue1);

    const auto queues = tracker.get_queues();
    REQUIRE(queues.size() == 1);
    REQUIRE(queues[0] == queue1);
}
