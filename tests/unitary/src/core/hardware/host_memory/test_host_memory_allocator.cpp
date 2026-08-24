// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <core/hardware/host_memory/host_memory_allocator.hpp>

#include <rexlib/core/hardware/memory_resource.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/system/host.hpp>

#include "../mock/mock_command_queue.hpp"

#include <sstream>

using namespace rexlib;

TEST_CASE( "get_max_alignment in host_memory_allocator should return the page size", "[host_memory_allocator]" )
{
	host_memory_allocator allocator;
	CHECK( allocator.get_max_alignment() == get_page_size() );
}

TEST_CASE( "get_memory_resource in host_memory_allocator should return the host memory resource", "[host_memory_allocator]" )
{
	host_memory_allocator allocator;
	CHECK( &allocator.get_memory_resource() == &get_host_memory_resource() );
}

TEST_CASE( "host_memory_allocator should return a host accessible buffer", "[host_memory_allocator]" )
{
	host_memory_allocator allocator;

	const std::size_t size = 1024;
	const std::size_t alignment = 64;
	auto buffer = allocator.allocate(size, alignment, nullptr);

	REQUIRE( buffer != nullptr );
	REQUIRE( reinterpret_cast<std::uintptr_t>(buffer->get_host_ptr()) % alignment == 0 );
	REQUIRE( buffer->get_size() >= size );
}

TEST_CASE( "host_memory_allocator should ignore the queue", "[host_memory_allocator]" )
{
	host_memory_allocator allocator;

	mock_command_queue queue;
	const std::size_t size = 1024;
	const std::size_t alignment = 64;
	allocator.allocate(size, alignment, &queue);
}

TEST_CASE( "host_memory_resource should return the host memory resource in get_memory_resource", "[host_memory_allocator]" )
{
	host_memory_allocator allocator;

	auto& resource1 = allocator.get_memory_resource();
	auto& resource2 = get_host_memory_resource();

	REQUIRE( &resource1 == &resource2 );
}

TEST_CASE( "host_buffer_allocator should allocate enough space", "[host_memory_allocator]" )
{
	const std::size_t size = 1024;
	const std::size_t alignment = 64;

	host_memory_allocator allocator;
	auto buffer = allocator.allocate(size, alignment, nullptr);

	REQUIRE( buffer->get_size() >= size );
	REQUIRE( reinterpret_cast<std::uintptr_t>(buffer->get_host_ptr()) % alignment == 0 );

	std::memset(buffer->get_host_ptr(), 0, buffer->get_size()); // Should not segfault
}
