// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include "test_fixture.hpp"

#include <rexlib/core/hardware/command.hpp>
#include <rexlib/core/hardware/command_queue.hpp>
#include <rexlib/core/hardware/device.hpp>
#include <rexlib/core/hardware/program.hpp>

#include <memory>
#include <stdexcept>

using namespace rexlib;

TEST_CASE(
	"a CUDA device should create queues that are not each other",
	"[cuda][queue]"
)
{
	const cuda::cuda_fixture fixture;
	if (!fixture.has_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	const auto first = fixture.get_device().create_command_queue();
	const auto second = fixture.get_device().create_command_queue();

	REQUIRE( first != nullptr );
	REQUIRE( second != nullptr );
	CHECK( first != second );
}

TEST_CASE(
	"a CUDA queue should refuse a command it cannot run",
	"[cuda][queue]"
)
{
	const cuda::cuda_fixture fixture;
	if (!fixture.has_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	const auto queue = fixture.get_device().create_command_queue();
	REQUIRE( queue != nullptr );

	SECTION( "when it has no program" )
	{
		CHECK_THROWS_AS( queue->submit(command()), std::invalid_argument );
	}

	SECTION( "when its program was not made by this backend" )
	{
		// The backend makes no program yet, so this is every program there
		// is.
		CHECK_THROWS_AS(
			queue->submit(command(std::make_shared<rexlib::program>())),
			std::invalid_argument
		);
	}
}
