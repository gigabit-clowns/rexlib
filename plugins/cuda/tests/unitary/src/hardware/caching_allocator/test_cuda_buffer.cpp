// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <hardware/caching_allocator/cuda_buffer.hpp>

#include <hardware/caching_allocator/caching_memory_allocator.hpp>

#include "../mock/mock_command_timeline.hpp"
#include "mock/mock_buffer.hpp"
#include "test_caching_allocator.hpp"

#include <config.hpp>

#include <rexlib/core/hardware/command_token.hpp>

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <thread>

using namespace rexlib;

namespace
{

constexpr std::size_t alignment =
	REXLIB_CUDA_CACHING_ALLOCATOR_MAX_ALIGNMENT_BYTES;
constexpr std::size_t min_heap =
	REXLIB_CUDA_CACHING_ALLOCATOR_MIN_HEAP_BYTES;

cuda::cuda_buffer& allocate_buffer(
	cuda::caching_allocator_fixture &fixture,
	std::size_t size,
	std::shared_ptr<rexlib::buffer> &owner
)
{
	command_token pending;
	owner = fixture.get().allocate_for_timeline(
		size, alignment, nullptr, pending
	);
	return cuda::cuda_buffer::cast(*owner);
}

} // anonymous namespace

TEST_CASE(
	"a fresh buffer should have no commands recorded against it",
	"[buffer]"
)
{
	cuda::caching_allocator_fixture fixture;
	std::shared_ptr<rexlib::buffer> owner;
	auto &buf = allocate_buffer(fixture, 1024, owner);

	CHECK( buf.get_recorded_uses().empty() );
}

TEST_CASE(
	"a buffer over a block that a command still uses should give it back "
	"carrying that token",
	"[buffer]"
)
{
	cuda::caching_allocator_fixture fixture;
	const auto &timeline = fixture.get_blocks().get_timeline(0);

	// A whole heap, dropped while a command still uses it.
	{
		std::shared_ptr<rexlib::buffer> owner;
		allocate_buffer(fixture, min_heap, owner)
			.record_use(command_token(timeline, 6));
	}

	// Handed out again and dropped without ever being used. The command is
	// still running against that memory all the same.
	{
		std::shared_ptr<rexlib::buffer> owner;
		const auto &buf = allocate_buffer(fixture, min_heap, owner);

		const auto uses = buf.get_recorded_uses();
		REQUIRE( uses.size() == 1 );
		CHECK( uses[0].get_timeline() == timeline );
		CHECK( uses[0].get_id() == 6 );
	}

	command_token pending;
	auto last = fixture.get().allocate_for_timeline(
		min_heap, alignment, nullptr, pending
	);

	CHECK( pending.get_timeline() == timeline );
	CHECK( pending.get_id() == 6 );
}

TEST_CASE(
	"a buffer should remember the last command of each timeline that uses it",
	"[buffer]"
)
{
	cuda::caching_allocator_fixture fixture;
	const auto &first = fixture.get_blocks().get_timeline(0);
	const auto &second = fixture.get_blocks().get_timeline(1);

	std::shared_ptr<rexlib::buffer> owner;
	auto &buf = allocate_buffer(fixture, 1024, owner);

	SECTION( "one token per timeline" )
	{
		buf.record_use(command_token(first, 1));
		buf.record_use(command_token(second, 1));

		const auto uses = buf.get_recorded_uses();
		REQUIRE( uses.size() == 2 );
		CHECK( uses[0].get_timeline() == first );
		CHECK( uses[1].get_timeline() == second );
	}

	SECTION( "keeping the later one, in whatever order they are recorded" )
	{
		// A stream finishes its commands in order, so waiting for the later
		// one is waiting for all of them.
		buf.record_use(command_token(first, 1));
		buf.record_use(command_token(first, 5));
		buf.record_use(command_token(first, 3));

		const auto uses = buf.get_recorded_uses();
		REQUIRE( uses.size() == 1 );
		CHECK( uses[0].get_timeline() == first );
		CHECK( uses[0].get_id() == 5 );
	}

	SECTION( "and nothing for a command that had finished on submission" )
	{
		buf.record_use(command_token());

		CHECK( buf.get_recorded_uses().empty() );
	}
}

TEST_CASE(
	"a buffer should take uses recorded from several threads at once",
	"[buffer]"
)
{
	cuda::caching_allocator_fixture fixture;
	const auto &first = fixture.get_blocks().get_timeline(0);
	const auto &second = fixture.get_blocks().get_timeline(1);

	std::shared_ptr<rexlib::buffer> owner;
	auto &buf = allocate_buffer(fixture, 1024, owner);

	const std::size_t count = 1000;
	const auto record = [&buf, count] (
		const std::shared_ptr<cuda::mock_command_timeline> &timeline
	)
	{
		for (std::size_t id = 1; id <= count; ++id)
		{
			buf.record_use(command_token(timeline, id));
		}
	};

	// Two queues may read one buffer at the same time.
	std::thread first_thread(record, first);
	std::thread second_thread(record, second);
	first_thread.join();
	second_thread.join();

	const auto uses = buf.get_recorded_uses();
	REQUIRE( uses.size() == 2 );
	CHECK( uses[0].get_id() == count );
	CHECK( uses[1].get_id() == count );
}

TEST_CASE(
	"a buffer that commands of several timelines still use should not be "
	"handed out again until one of them is left",
	"[buffer]"
)
{
	cuda::caching_allocator_fixture fixture;
	auto &allocator = fixture.get();
	const auto &first = fixture.get_blocks().get_timeline(0);
	const auto &second = fixture.get_blocks().get_timeline(1);

	// A whole heap at a time, so that the block being held back is the only
	// thing that could serve the next request.
	void *first_data = nullptr;
	{
		std::shared_ptr<rexlib::buffer> owner;
		auto &buf = allocate_buffer(fixture, min_heap, owner);
		first_data = buf.get_device_ptr();
		buf.record_use(command_token(first, 1));
		buf.record_use(command_token(second, 1));
	}

	// Held back rather than given straight to the pool, so the next request
	// has to be served out of memory taken from the driver instead.
	command_token pending;
	auto other =
		allocator.allocate_for_timeline(min_heap, alignment, nullptr, pending);
	CHECK( cuda::cuda_buffer::cast(*other).get_device_ptr() != first_data );
	CHECK( fixture.get_blocks().get_arena().get_used_bytes() == 2 * min_heap );

	// Once one of the two commands has finished, the block goes back into
	// the pool carrying the token of the other.
	other.reset();
	ALLOW_CALL(*first, is_complete(1u))
		.RETURN(true);
	FORBID_CALL(fixture.get_blocks().get_source(), allocate(trompeloeil::_));

	command_token unused_pending;
	auto unused = allocator.allocate_for_timeline(
		min_heap, alignment, nullptr, unused_pending
	);
	command_token held_pending;
	auto held = allocator.allocate_for_timeline(
		min_heap, alignment, nullptr, held_pending
	);

	// The memory nothing uses is handed out first.
	CHECK( unused_pending.is_empty() );
	CHECK( cuda::cuda_buffer::cast(*held).get_device_ptr() == first_data );
	CHECK( held_pending.get_timeline() == second );
	CHECK( held_pending.get_id() == 1 );
}

TEST_CASE(
	"a buffer that a single command still uses should go straight back",
	"[buffer]"
)
{
	cuda::caching_allocator_fixture fixture;
	const auto &timeline = fixture.get_blocks().get_timeline(0);

	void *first_data = nullptr;
	{
		std::shared_ptr<rexlib::buffer> owner;
		auto &buf = allocate_buffer(fixture, 1024, owner);
		first_data = buf.get_device_ptr();
		buf.record_use(command_token(timeline, 2));
	}

	// A queue waits for its own commands by running in order, so the block
	// is its to have again at once.
	command_token pending;
	auto second = fixture.get().allocate_for_timeline(
		1024, alignment, timeline.get(), pending
	);

	CHECK( cuda::cuda_buffer::cast(*second).get_device_ptr() == first_data );
	CHECK( pending.get_timeline() == timeline );
	CHECK( pending.get_id() == 2 );
}

TEST_CASE(
	"casting a buffer should reject one this backend did not make",
	"[buffer]"
)
{
	cuda::caching_allocator_fixture fixture;
	std::shared_ptr<rexlib::buffer> owner;
	auto &buf = allocate_buffer(fixture, 1024, owner);

	CHECK( &cuda::cuda_buffer::cast(*owner) == &buf );

	cuda::mock_buffer foreign;
	CHECK_THROWS_AS( cuda::cuda_buffer::cast(foreign), std::invalid_argument );

	const auto &const_foreign = foreign;
	CHECK_THROWS_AS(
		cuda::cuda_buffer::cast(const_foreign),
		std::invalid_argument
	);
}
