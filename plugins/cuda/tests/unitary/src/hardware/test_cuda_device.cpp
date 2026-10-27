// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <hardware/cuda_device.hpp>

#include <hardware/cuda_command_queue.hpp>

#include "test_driver.hpp"

#include <rexlib/core/hardware/memory_resource.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/hardware/memory_resource_kind.hpp>

#include <stdexcept>

using namespace rexlib;

TEST_CASE(
	"a device should report the ordinal it was built with",
	"[device]"
)
{
	const cuda::cuda_device dev(0);

	CHECK( dev.get_ordinal() == 0 );
}

TEST_CASE(
	"a device should offer memory for either side",
	"[device]"
)
{
	const cuda::cuda_device dev(0);

	CHECK(
		is_device_accessible(
			dev.get_memory_resource(memory_resource_affinity::device)
				.get_kind()
		)
	);
	CHECK(
		is_host_accessible(
			dev.get_memory_resource(memory_resource_affinity::host).get_kind()
		)
	);
}

TEST_CASE(
	"a device should refuse an affinity it does not know",
	"[device]"
)
{
	const cuda::cuda_device dev(0);

	// Only where the two affinities are answered by different resources is
	// there anything to choose between.
	if (&dev.get_memory_resource(memory_resource_affinity::device) ==
	    &dev.get_memory_resource(memory_resource_affinity::host))
	{
		SKIP( "The device shares the memory of the host." );
	}

	CHECK_THROWS_AS(
		dev.get_memory_resource(static_cast<memory_resource_affinity>(-1)),
		std::invalid_argument
	);
}

TEST_CASE(
	"a device should create queues that are independent of each other",
	"[device]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	const cuda::cuda_device dev(0);

	const auto first = dev.create_command_queue();
	const auto second = dev.create_command_queue();
	REQUIRE( first != nullptr );
	REQUIRE( second != nullptr );

	const auto &first_queue = cuda::cuda_command_queue::cast(*first);
	const auto &second_queue = cuda::cuda_command_queue::cast(*second);

	CHECK( first_queue.get_ordinal() == 0 );
	CHECK( first_queue.get_handle() != second_queue.get_handle() );
	CHECK( first_queue.get_timeline() != second_queue.get_timeline() );
}
