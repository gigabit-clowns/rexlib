// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <hardware/event.hpp>

#include "test_driver.hpp"
#include "test_stream_blocker.hpp"

#include <cuda_runtime.h>

using namespace rexlib;

TEST_CASE(
	"an event should belong to the device it was created on",
	"[event]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	const cuda::event ev(0);

	CHECK( ev.get_ordinal() == 0 );
	CHECK( ev.get_handle() != nullptr );
}

TEST_CASE(
	"an event that was never recorded should count as reached",
	"[event]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	const cuda::event ev(0);

	CHECK( ev.is_signaled() );
	CHECK_NOTHROW( ev.wait() );
}

TEST_CASE(
	"an event should be reached once its stream gets to where it was "
	"recorded",
	"[event]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	cudaStream_t stream = nullptr;
	REQUIRE(
		cudaStreamCreateWithFlags(&stream, cudaStreamNonBlocking) ==
		cudaSuccess
	);

	{
		const cuda::event ev(0);
		cuda::stream_blocker blocker(stream);
		REQUIRE( cudaEventRecord(ev.get_handle(), stream) == cudaSuccess );

		CHECK_FALSE( ev.is_signaled() );

		blocker.release();
		ev.wait();

		CHECK( ev.is_signaled() );
	}

	REQUIRE( cudaStreamDestroy(stream) == cudaSuccess );
}
