// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <hardware/caching_allocator/caching_memory_allocator.hpp>

#include <hardware/caching_allocator/buffer.hpp>
#include <hardware/command_queue.hpp>
#include <hardware/pooled_event_recorder.hpp>
#include <hardware/stream_timeline.hpp>

#include "../mock/mock_command_queue.hpp"
#include "../mock/mock_command_timeline.hpp"
#include "../test_driver.hpp"
#include "../test_stream_blocker.hpp"
#include "mock/mock_memory_resource.hpp"
#include "test_caching_allocator.hpp"

#include <config.hpp>

#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/hardware/command_token.hpp>
#include <rexlib/core/memory/align.hpp>

#include <cstddef>
#include <memory>
#include <stdexcept>

using namespace rexlib;

namespace
{

constexpr std::size_t alignment =
	REXLIB_CUDA_CACHING_ALLOCATOR_MAX_ALIGNMENT_BYTES;
constexpr std::size_t min_heap =
	REXLIB_CUDA_CACHING_ALLOCATOR_MIN_HEAP_BYTES;

} // anonymous namespace

TEST_CASE(
	"caching_memory_allocator should report what it is backed by",
	"[caching_memory_allocator]"
)
{
	cuda::caching_allocator_fixture fixture;

	CHECK( &fixture.get().get_memory_resource() == &fixture.get_resource() );

	// Whatever the blocks underneath can be aligned to is what a buffer can
	// be, since a buffer is one of those blocks.
	CHECK(
		fixture.get().get_max_alignment() ==
		fixture.get_blocks().get().get_max_alignment()
	);
}

TEST_CASE(
	"caching_memory_allocator should refuse to be built without blocks to "
	"hand out",
	"[caching_memory_allocator]"
)
{
	cuda::mock_memory_resource resource;

	CHECK_THROWS_AS(
		cuda::caching_memory_allocator(resource, nullptr),
		std::invalid_argument
	);
}

TEST_CASE(
	"a caching_memory_allocator should allocate whatever the hint is",
	"[caching_memory_allocator]"
)
{
	cuda::caching_allocator_fixture fixture;
	command_token pending;

	SECTION( "when there is none" )
	{
		auto buf = fixture.get().allocate(1024, alignment, nullptr, pending);

		REQUIRE( buf != nullptr );
		CHECK( pending.is_empty() );
	}

	SECTION( "when it is a queue this backend did not make" )
	{
		// A hint only says where the buffer is expected to be used first. A
		// queue with nothing to prefer is still a queue the buffer can be
		// used on.
		cuda::mock_command_queue queue;
		auto buf = fixture.get().allocate(1024, alignment, &queue, pending);

		REQUIRE( buf != nullptr );
		CHECK( pending.is_empty() );
	}
}

TEST_CASE(
	"a buffer from a caching_memory_allocator should describe the block it "
	"was cut from",
	"[caching_memory_allocator]"
)
{
	cuda::caching_allocator_fixture fixture;

	command_token pending;
	auto buf = fixture.get().allocate(1000, alignment, nullptr, pending);

	REQUIRE( buf != nullptr );
	CHECK( &buf->get_memory_resource() == &fixture.get_resource() );

	// Rounding up is what keeps the remainder of a block usable, so the size
	// the buffer reports is the rounded one rather than what was asked for.
	CHECK( buf->get_size() == align_ceil(std::size_t(1000), alignment) );
	CHECK( is_aligned(cuda::buffer::cast(*buf).get_device_ptr(), alignment) );
}

TEST_CASE(
	"a buffer from a caching_memory_allocator should only offer a host "
	"pointer when the host can reach it",
	"[caching_memory_allocator]"
)
{
	command_token pending;

	SECTION( "not when it is device local" )
	{
		cuda::caching_allocator_fixture fixture(false);

		auto buf = fixture.get().allocate(1024, alignment, nullptr, pending);
		CHECK( buf->get_host_ptr() == nullptr );
		CHECK( cuda::buffer::cast(*buf).get_device_ptr() != nullptr );
	}

	SECTION( "but yes when it is page locked host memory" )
	{
		cuda::caching_allocator_fixture fixture(true);

		auto buf = fixture.get().allocate(1024, alignment, nullptr, pending);
		CHECK( buf->get_host_ptr() != nullptr );
		CHECK(
			buf->get_host_ptr() == cuda::buffer::cast(*buf).get_device_ptr()
		);
	}
}

TEST_CASE(
	"a buffer from a caching_memory_allocator should give its block back",
	"[caching_memory_allocator]"
)
{
	cuda::caching_allocator_fixture fixture;
	command_token pending;

	void *first = nullptr;
	{
		auto buf =
			fixture.get().allocate(min_heap, alignment, nullptr, pending);
		first = cuda::buffer::cast(*buf).get_device_ptr();
	}

	// Only reachable again if dropping the buffer put the block back, since a
	// whole heap's worth leaves nothing else to cut this out of.
	FORBID_CALL(fixture.get_blocks().get_source(), allocate(trompeloeil::_));
	auto second =
		fixture.get().allocate(min_heap, alignment, nullptr, pending);
	CHECK( cuda::buffer::cast(*second).get_device_ptr() == first );
}

TEST_CASE(
	"a caching_memory_allocator should hand out the token of what still uses "
	"the memory",
	"[caching_memory_allocator]"
)
{
	cuda::caching_allocator_fixture fixture;
	auto &allocator = fixture.get();
	const auto &timeline = fixture.get_blocks().get_timeline(0);

	// A whole heap, dropped while a command still uses it.
	void *data = nullptr;
	{
		command_token pending;
		auto buf = allocator.allocate(min_heap, alignment, nullptr, pending);

		// Nothing has ever run against new memory.
		CHECK( pending.is_empty() );

		data = cuda::buffer::cast(*buf).get_device_ptr();
		cuda::buffer::cast(*buf).record_use(command_token(timeline, 6));
	}

	SECTION( "while that command is running" )
	{
		command_token pending;
		auto buf = allocator.allocate(min_heap, alignment, nullptr, pending);

		CHECK( cuda::buffer::cast(*buf).get_device_ptr() == data );
		CHECK( pending.get_timeline() == timeline );
		CHECK( pending.get_id() == 6 );
	}

	SECTION( "and an empty one once it has finished" )
	{
		ALLOW_CALL(*timeline, is_complete(6u))
			.RETURN(true);

		command_token pending;
		auto buf = allocator.allocate(min_heap, alignment, nullptr, pending);

		CHECK( cuda::buffer::cast(*buf).get_device_ptr() == data );
		CHECK( pending.is_empty() );
	}

	SECTION( "keeping it on the block when the command cannot be asked about" )
	{
		command_token pending;
		{
			ALLOW_CALL(*timeline, is_complete(6u))
				.THROW(std::runtime_error("no answer"));

			CHECK_THROWS_AS(
				allocator.allocate(min_heap, alignment, nullptr, pending),
				std::runtime_error
			);
		}

		// The block went back with what it was carrying, rather than as
		// memory nothing uses.
		auto buf = allocator.allocate(min_heap, alignment, nullptr, pending);

		CHECK( cuda::buffer::cast(*buf).get_device_ptr() == data );
		CHECK( pending.get_timeline() == timeline );
		CHECK( pending.get_id() == 6 );
	}
}

TEST_CASE(
	"a caching_memory_allocator should prefer the memory the named timeline "
	"can use at no cost",
	"[caching_memory_allocator]"
)
{
	cuda::caching_allocator_fixture fixture;
	auto &allocator = fixture.get();
	const auto &first = fixture.get_blocks().get_timeline(0);
	const auto &second = fixture.get_blocks().get_timeline(1);

	// Two halves of one heap, each dropped while a command of another
	// timeline still uses it.
	void *second_data = nullptr;
	{
		command_token pending;
		auto first_buffer =
			allocator.allocate(min_heap / 2, alignment, nullptr, pending);
		auto second_buffer =
			allocator.allocate(min_heap / 2, alignment, nullptr, pending);

		second_data = cuda::buffer::cast(*second_buffer).get_device_ptr();
		cuda::buffer::cast(*first_buffer).record_use(command_token(first, 1));
		cuda::buffer::cast(*second_buffer).record_use(command_token(second, 1));
	}

	command_token pending;
	auto buf = allocator.allocate_for_timeline(
		min_heap / 2, alignment, second.get(), pending
	);

	CHECK( cuda::buffer::cast(*buf).get_device_ptr() == second_data );
	CHECK( pending.get_timeline() == second );
}

TEST_CASE(
	"a caching_memory_allocator should prefer the memory the hinted queue "
	"can use at no cost",
	"[caching_memory_allocator]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	cuda::caching_allocator_fixture fixture;
	auto &allocator = fixture.get();
	const auto &other = fixture.get_blocks().get_timeline(0);

	cuda::command_queue queue(
		0,
		std::make_shared<cuda::pooled_event_recorder>(0)
	);

	// Declared after the allocator, so that the stream is let go before the
	// allocator waits for it on its way out.
	cuda::stream_blocker blocker(queue.get_handle());
	const command_token own(
		queue.get_timeline(),
		queue.get_timeline()->record(queue.get_handle())
	);

	// Two halves of one heap, one dropped while a command of the queue still
	// uses it and the other while a command of another timeline does.
	void *own_data = nullptr;
	{
		command_token pending;
		auto other_buffer =
			allocator.allocate(min_heap / 2, alignment, nullptr, pending);
		auto own_buffer =
			allocator.allocate(min_heap / 2, alignment, nullptr, pending);

		own_data = cuda::buffer::cast(*own_buffer).get_device_ptr();
		cuda::buffer::cast(*other_buffer).record_use(command_token(other, 1));
		cuda::buffer::cast(*own_buffer).record_use(own);
	}

	command_token pending;
	auto buf = allocator.allocate(min_heap / 2, alignment, &queue, pending);

	CHECK( cuda::buffer::cast(*buf).get_device_ptr() == own_data );
	CHECK( pending.get_timeline() == queue.get_timeline() );
	CHECK( pending.get_id() == own.get_id() );
}

TEST_CASE(
	"caching_memory_allocator should pass a trim down to its blocks",
	"[caching_memory_allocator]"
)
{
	cuda::caching_allocator_fixture fixture;

	command_token pending;
	fixture.get().allocate(1024, alignment, nullptr, pending);
	REQUIRE( fixture.get_blocks().get_arena().get_used_bytes() == min_heap );

	CHECK( fixture.get().trim() == min_heap );
	CHECK( fixture.get_blocks().get_arena().get_region_count() == 0 );
}
