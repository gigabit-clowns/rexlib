// SPDX-License-Identifier: GPL-3.0-only

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
