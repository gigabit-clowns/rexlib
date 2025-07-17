// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/cuda/compute/allocator/cuda_memory_block.hpp>

#include <cstddef>

#include <catch2/catch_test_macros.hpp>

using namespace xmipp4::compute;

TEST_CASE( "construct cuda_memory_block", "[cuda_memory_block]" )
{
    const std::uintptr_t ptr_value = 0xDEADBEEF;
    auto *const ptr = reinterpret_cast<void*>(ptr_value);
    const std::uintptr_t queue_value = 0xA7EBADF0D;
    auto *const queue = reinterpret_cast<cuda_device_queue*>(queue_value);

    cuda_memory_block block(ptr, 0xC0FFE, queue);
    REQUIRE( block.get_data() == ptr );
    REQUIRE( block.get_size() == 0xC0FFE );
    REQUIRE( block.get_queue() == queue );
}
