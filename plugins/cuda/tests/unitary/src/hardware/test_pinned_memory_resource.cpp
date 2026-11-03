// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <hardware/pinned_memory_resource.hpp>

#include <rexlib/core/hardware/memory_allocator.hpp>

using namespace rexlib;

TEST_CASE(
	"the pinned_memory_resource should be the one of the whole process",
	"[pinned_memory_resource]"
)
{
	// Page locking is done to host memory, which belongs to the process rather
	// than to any one device, so there is nothing to have more than one of.
	CHECK(
		&cuda::pinned_memory_resource::get() ==
		&cuda::pinned_memory_resource::get()
	);
}

TEST_CASE(
	"the pinned_memory_resource should describe memory the host can reach",
	"[pinned_memory_resource]"
)
{
	const auto &resource = cuda::pinned_memory_resource::get();
	const auto kind = resource.get_kind();

	CHECK( is_host_accessible(kind) );

	// Staging for a transfer, unless the devices share the host's memory, in
	// which case it is what they compute out of and saying otherwise would be
	// describing hardware that is not there.
	CHECK(
		(kind == memory_resource_kind::host_staging ||
		 kind == memory_resource_kind::unified)
	);
	if (kind == memory_resource_kind::unified)
	{
		CHECK( is_device_accessible(kind) );
	}
}

TEST_CASE(
	"the pinned_memory_resource should hand out a cache of its own each time",
	"[pinned_memory_resource]"
)
{
	const auto &resource = cuda::pinned_memory_resource::get();

	auto first = resource.create_allocator();
	auto second = resource.create_allocator();

	REQUIRE( first != nullptr );
	REQUIRE( second != nullptr );
	CHECK( first != second );

	CHECK( &first->get_memory_resource() == &resource );
}
