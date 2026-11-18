// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <hardware/pooled_event_recorder.hpp>

#include "test_driver.hpp"
#include "test_stream_blocker.hpp"

#include <array>
#include <cstddef>

#include <cuda_runtime.h>

using namespace rexlib;

namespace
{

/// Streams of the first device, destroyed with the test.
class stream_fixture
{
public:
	stream_fixture()
	{
		for (auto &stream : m_streams)
		{
			stream = nullptr;
			REQUIRE(
				cudaStreamCreateWithFlags(&stream, cudaStreamNonBlocking) ==
				cudaSuccess
			);
		}
	}

	stream_fixture(const stream_fixture &other) = delete;
	stream_fixture(stream_fixture &&other) = delete;

	~stream_fixture()
	{
		for (const auto stream : m_streams)
		{
			cudaStreamDestroy(stream);
		}
	}

	stream_fixture& operator=(const stream_fixture &other) = delete;
	stream_fixture& operator=(stream_fixture &&other) = delete;

	cudaStream_t get(std::size_t index) const noexcept
	{
		return m_streams[index];
	}

private:
	std::array<cudaStream_t, 2> m_streams;
};

} // anonymous namespace

TEST_CASE(
	"pooled_event_recorder should capture a point that is reached once the "
	"stream gets there",
	"[pooled_event_recorder]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	const stream_fixture streams;
	cuda::pooled_event_recorder recorder(0);
	cuda::stream_blocker blocker(streams.get(0));

	const auto ticket = recorder.record(streams.get(0));

	CHECK( ticket != cuda::event_recorder::no_ticket );
	CHECK_FALSE( recorder.is_complete(ticket) );

	blocker.release();
	recorder.wait(ticket);

	CHECK( recorder.is_complete(ticket) );

	recorder.release(ticket);
}

TEST_CASE(
	"pooled_event_recorder should capture a point of an idle stream that is "
	"reached at once",
	"[pooled_event_recorder]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	const stream_fixture streams;
	cuda::pooled_event_recorder recorder(0);

	const auto ticket = recorder.record(streams.get(0));
	recorder.wait(ticket);

	CHECK( recorder.is_complete(ticket) );

	recorder.release(ticket);
}

TEST_CASE(
	"pooled_event_recorder should hand a ticket out again once it is given "
	"back",
	"[pooled_event_recorder]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	const stream_fixture streams;
	cuda::pooled_event_recorder recorder(0);

	const auto first = recorder.record(streams.get(0));
	const auto second = recorder.record(streams.get(0));

	// Two points that are both held are two events.
	CHECK( first != second );

	// Recycling the event is the whole point of the pool.
	recorder.release(first);
	CHECK( recorder.record(streams.get(0)) == first );

	recorder.release(first);
	recorder.release(second);
}

TEST_CASE(
	"pooled_event_recorder should ignore being given no ticket back",
	"[pooled_event_recorder]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	const stream_fixture streams;
	cuda::pooled_event_recorder recorder(0);

	recorder.release(cuda::event_recorder::no_ticket);

	// Nothing went into the pool, so the first ticket is still the first.
	const auto ticket = recorder.record(streams.get(0));
	recorder.release(cuda::event_recorder::no_ticket);
	recorder.release(ticket);
	CHECK( recorder.record(streams.get(0)) == ticket );

	recorder.release(ticket);
}

TEST_CASE(
	"pooled_event_recorder should make a stream wait for a point of another",
	"[pooled_event_recorder]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	const stream_fixture streams;
	cuda::pooled_event_recorder recorder(0);
	cuda::stream_blocker blocker(streams.get(0));

	const auto held = recorder.record(streams.get(0));
	recorder.enqueue_wait(streams.get(1), held);
	const auto waiting = recorder.record(streams.get(1));

	// The second stream has nothing to do but wait for the first one.
	CHECK_FALSE( recorder.is_complete(waiting) );

	blocker.release();
	recorder.wait(waiting);

	CHECK( recorder.is_complete(held) );
	CHECK( recorder.is_complete(waiting) );

	recorder.release(held);
	recorder.release(waiting);
}
