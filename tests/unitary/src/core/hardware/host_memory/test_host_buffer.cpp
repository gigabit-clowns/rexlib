// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <core/hardware/host_memory/host_buffer.hpp>

#include <rex/core/exceptions/invalid_operation_error.hpp>
#include <core/hardware/host_memory/host_memory_resource.hpp>
#include <core/hardware/host_memory/host_memory_allocator.hpp>

#include <sstream>

using namespace rex;

TEST_CASE( "host_buffer should allocate enough space", "[host_buffer]" )
{
	const std::size_t size = 1024;
	const std::size_t alignment = 64;

	host_buffer buffer(size, alignment);

	REQUIRE( buffer.get_host_ptr() );
	REQUIRE( buffer.get_size() >= size );
	REQUIRE( reinterpret_cast<std::uintptr_t>(buffer.get_host_ptr()) % alignment == 0 );
	std::memset(buffer.get_host_ptr(), 0, size); // Should not segfault
}

TEST_CASE( "host_buffer should return the host memory resource singleton in get_memory_resource", "[host_buffer]" )
{
	const std::size_t size = 1024;
	const std::size_t alignment = 64;

	host_buffer buffer(size, alignment);
	auto &mr1 = buffer.get_memory_resource();
	auto &mr2 = host_memory_resource::get();

	REQUIRE( &mr1 == &mr2 );
}
