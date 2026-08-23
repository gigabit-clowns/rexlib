// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <core/hardware/host_memory/host_memory_resource.hpp>

#include <core/hardware/host_memory/host_memory_allocator.hpp>

using namespace rex;

TEST_CASE( "host_memory_resource should be a singleton", "[host_memory_resource]" )
{
	auto &a = host_memory_resource::get();
	auto &b = host_memory_resource::get();
	REQUIRE( &a == &b );
}

TEST_CASE( "host_memory_resource should be of host kind", "[host_memory_resource]" )
{
	const auto& resource = host_memory_resource::get();
	REQUIRE( resource.get_kind() == memory_resource_kind::host );
}

TEST_CASE( "host_memory_resource should create a valid allocator", "[host_memory_resource]" )
{
	const auto& resource = host_memory_resource::get();
	const auto allocator = resource.create_allocator();

	REQUIRE( allocator );
	REQUIRE( allocator == host_memory_allocator::create() );
}
