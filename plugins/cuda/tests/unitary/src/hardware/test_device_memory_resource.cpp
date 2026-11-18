// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <hardware/device_memory_resource.hpp>

#include <rexlib/core/hardware/memory_allocator.hpp>

using namespace rexlib;

TEST_CASE(
	"a device_memory_resource should describe the memory local to a device",
	"[device_memory_resource]"
)
{
	const cuda::device_memory_resource resource(3);

	CHECK( resource.get_ordinal() == 3 );
	CHECK( resource.get_kind() == memory_resource_kind::device_local );

	// The host cannot reach it, which is what makes it worth having a separate
	// page locked resource to stage transfers through.
	CHECK( is_device_accessible(resource.get_kind()) );
	CHECK_FALSE( is_host_accessible(resource.get_kind()) );
}

TEST_CASE(
	"a device_memory_resource should hand out a cache of its own each time",
	"[device_memory_resource]"
)
{
	const cuda::device_memory_resource resource(0);

	auto first = resource.create_allocator();
	auto second = resource.create_allocator();

	REQUIRE( first != nullptr );
	REQUIRE( second != nullptr );

	// Sharing a cache means sharing the allocator, which is what a device
	// context is for. Asking again here is asking for another cache.
	CHECK( first != second );

	CHECK( &first->get_memory_resource() == &resource );
	CHECK( &second->get_memory_resource() == &resource );
}
