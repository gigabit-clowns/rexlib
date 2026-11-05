// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <core/hardware/host_memory/host_memory_allocator.hpp>

#include <rexlib/core/hardware/memory_resource.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/system/host.hpp>

#include "../mock/mock_command_queue.hpp"
#include "../mock/mock_command_timeline.hpp"

#include <memory>
#include <stdexcept>
#include <sstream>

using namespace rexlib;

TEST_CASE(
	"get_max_alignment in host_memory_allocator should return the page size",
	"[host_memory_allocator]"
)
{
	host_memory_allocator allocator;
	CHECK( allocator.get_max_alignment() == get_page_size() );
}

TEST_CASE(
	"get_memory_resource in host_memory_allocator should return the host "
	"memory resource",
	"[host_memory_allocator]"
)
{
	host_memory_allocator allocator;
	CHECK( &allocator.get_memory_resource() == &get_host_memory_resource() );
}

TEST_CASE(
	"host_memory_allocator should return a host accessible buffer",
	"[host_memory_allocator]"
)
{
	host_memory_allocator allocator;

	const std::size_t size = 1024;
	const std::size_t alignment = 64;
	command_token pending;
	auto buffer = allocator.allocate(size, alignment, nullptr, pending);

	REQUIRE( buffer != nullptr );
	REQUIRE(
		reinterpret_cast<std::uintptr_t>(buffer->get_host_ptr()) % alignment ==
		0
	);
	REQUIRE( buffer->get_size() >= size );
}

TEST_CASE(
	"allocate in host_memory_allocator should throw when the alignment is not "
	"a power of two",
	"[host_memory_allocator]"
)
{
	const std::size_t alignment = GENERATE(as<std::size_t>{}, 0, 3, 48);

	host_memory_allocator allocator;
	command_token pending;
	CHECK_THROWS_AS(
		allocator.allocate(1024, alignment, nullptr, pending),
		std::invalid_argument
	);
}

TEST_CASE(
	"allocate in host_memory_allocator should throw when the alignment "
	"exceeds the maximum alignment",
	"[host_memory_allocator]"
)
{
	host_memory_allocator allocator;
	const std::size_t alignment = 2 * allocator.get_max_alignment();
	command_token pending;
	CHECK_THROWS_AS(
		allocator.allocate(1024, alignment, nullptr, pending),
		std::invalid_argument
	);
}

TEST_CASE(
	"host_memory_allocator should ignore the queue",
	"[host_memory_allocator]"
)
{
	host_memory_allocator allocator;

	mock_command_queue queue;
	const std::size_t size = 1024;
	const std::size_t alignment = 64;
	command_token pending;
	allocator.allocate(size, alignment, &queue, pending);
}

TEST_CASE(
	"host_memory_allocator should hand out an empty token",
	"[host_memory_allocator]"
)
{
	host_memory_allocator allocator;

	const std::size_t size = 1024;
	const std::size_t alignment = 64;
	command_token pending(std::make_shared<mock_command_timeline>(), 7);
	allocator.allocate(size, alignment, nullptr, pending);

	CHECK( pending.is_empty() );
}

TEST_CASE(
	"host_memory_resource should return the host memory resource in "
	"get_memory_resource",
	"[host_memory_allocator]"
)
{
	host_memory_allocator allocator;

	auto& resource1 = allocator.get_memory_resource();
	auto& resource2 = get_host_memory_resource();

	REQUIRE( &resource1 == &resource2 );
}

TEST_CASE(
	"host_buffer_allocator should allocate enough space",
	"[host_memory_allocator]"
)
{
	const std::size_t size = 1024;
	const std::size_t alignment = 64;

	host_memory_allocator allocator;
	command_token pending;
	auto buffer = allocator.allocate(size, alignment, nullptr, pending);

	REQUIRE( buffer->get_size() >= size );
	REQUIRE(
		reinterpret_cast<std::uintptr_t>(buffer->get_host_ptr()) % alignment ==
		0
	);

	// Should not segfault
	std::memset(buffer->get_host_ptr(), 0, buffer->get_size());
}
