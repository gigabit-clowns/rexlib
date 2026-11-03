// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <hardware/caching_allocator/memory_block_allocator.hpp>

#include <hardware/caching_allocator/memory_block.hpp>

#include "../mock/mock_command_timeline.hpp"
#include "mock/mock_memory_source.hpp"
#include "test_block_allocator.hpp"

#include <config.hpp>

#include <rexlib/core/hardware/command_token.hpp>
#include <rexlib/core/memory/align.hpp>
#include <rexlib/core/span.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <stdexcept>
#include <vector>

using namespace rexlib;

namespace
{

constexpr std::size_t alignment =
	REXLIB_CUDA_CACHING_ALLOCATOR_MAX_ALIGNMENT_BYTES;
constexpr std::size_t min_heap =
	REXLIB_CUDA_CACHING_ALLOCATOR_MIN_HEAP_BYTES;

/// Blocks a test holds on to, given back together at the end.
class held_blocks
{
public:
	explicit held_blocks(cuda::memory_block_allocator &allocator) noexcept
		: m_allocator(&allocator)
	{
	}

	held_blocks(const held_blocks &other) = delete;
	held_blocks(held_blocks &&other) = delete;

	~held_blocks()
	{
		for (auto *block : m_blocks)
		{
			m_allocator->recycle(*block, span<const command_token>());
		}
	}

	held_blocks& operator=(const held_blocks &other) = delete;
	held_blocks& operator=(held_blocks &&other) = delete;

	cuda::memory_block& take(
		std::size_t size,
		const command_timeline *timeline = nullptr
	)
	{
		auto &result = m_allocator->allocate(size, alignment, timeline);
		m_blocks.push_back(&result);
		return result;
	}

	const void* get_data(std::size_t index) const noexcept
	{
		return m_blocks[index]->get_data();
	}

private:
	cuda::memory_block_allocator *m_allocator;
	std::vector<cuda::memory_block*> m_blocks;
};

} // anonymous namespace

TEST_CASE(
	"memory_block_allocator should say what it is willing to do",
	"[memory_block_allocator]"
)
{
	SECTION( "for memory the host cannot address" )
	{
		cuda::block_allocator_fixture fixture(cuda::test_arena::unlimited, false);
		CHECK( fixture.get().get_max_alignment() == alignment );
		CHECK_FALSE( fixture.get().is_host_accessible() );
	}

	SECTION( "and for memory it can" )
	{
		cuda::block_allocator_fixture fixture(cuda::test_arena::unlimited, true);
		CHECK( fixture.get().is_host_accessible() );
	}
}

TEST_CASE(
	"memory_block_allocator should refuse to be built without a memory "
	"source",
	"[memory_block_allocator]"
)
{
	CHECK_THROWS_AS(
		cuda::memory_block_allocator(nullptr),
		std::invalid_argument
	);
}

TEST_CASE(
	"memory_block_allocator should refuse an alignment it cannot promise",
	"[memory_block_allocator]"
)
{
	cuda::block_allocator_fixture fixture;

	FORBID_CALL(fixture.get_source(), allocate(trompeloeil::_));

	SECTION( "when it is stricter than the maximum" )
	{
		CHECK_THROWS_AS(
			fixture.get().allocate(1024, 2 * alignment, nullptr),
			std::invalid_argument
		);
	}

	SECTION( "when it is not a power of two" )
	{
		CHECK_THROWS_AS(
			fixture.get().allocate(1024, 24, nullptr),
			std::invalid_argument
		);
	}

	SECTION( "when it is zero" )
	{
		CHECK_THROWS_AS(
			fixture.get().allocate(1024, 0, nullptr),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"a block from a memory_block_allocator should be aligned and at least as "
	"big as asked for",
	"[memory_block_allocator]"
)
{
	cuda::block_allocator_fixture fixture;
	held_blocks blocks(fixture.get());

	auto &block = blocks.take(1000);

	CHECK( is_aligned(block.get_data(), alignment) );

	// Rounding up is what keeps the remainder of a block usable, so the size
	// it reports is the rounded one rather than what was asked for.
	CHECK( block.get_size() == align_ceil(std::size_t(1000), alignment) );
}

TEST_CASE(
	"a block of new memory from a memory_block_allocator should carry an "
	"empty token",
	"[memory_block_allocator]"
)
{
	cuda::block_allocator_fixture fixture;
	held_blocks blocks(fixture.get());

	CHECK( blocks.take(1024).get_pending().is_empty() );
}

TEST_CASE(
	"a memory_block_allocator should stop reaching the driver once it has "
	"memory of its own",
	"[memory_block_allocator]"
)
{
	cuda::block_allocator_fixture fixture;
	const command_token token(fixture.get_timeline(0), 1);

	REQUIRE_CALL(fixture.get_source(), allocate(min_heap))
		.LR_RETURN(fixture.get_arena().take(_1));

	const auto *first = fixture.allocate_and_drop(1024, token);
	for (std::size_t i = 0; i < 1000; ++i)
	{
		// Handed straight back to the queue whose command still uses it,
		// which waits for it by running in order, so it lands in the same
		// place every time.
		CHECK( fixture.allocate_and_drop(1024, token) == first );
	}
}

TEST_CASE(
	"a memory_block_allocator should cut one heap into the blocks that are "
	"alive at once",
	"[memory_block_allocator]"
)
{
	cuda::block_allocator_fixture fixture;

	REQUIRE_CALL(fixture.get_source(), allocate(min_heap))
		.LR_RETURN(fixture.get_arena().take(_1));

	held_blocks blocks(fixture.get());
	for (std::size_t i = 0; i < 4; ++i)
	{
		blocks.take(1024);
	}

	// Consecutive, since each one is cut off the front of what the last one
	// left behind.
	for (std::size_t i = 1; i < 4; ++i)
	{
		const auto previous =
			reinterpret_cast<std::uintptr_t>(blocks.get_data(i - 1));
		const auto current =
			reinterpret_cast<std::uintptr_t>(blocks.get_data(i));
		CHECK( current == previous + 1024 );
	}
}

TEST_CASE(
	"a memory_block_allocator should grow by doubling",
	"[memory_block_allocator]"
)
{
	cuda::block_allocator_fixture fixture;

	trompeloeil::sequence sequence;

	// The first heap is the smallest one worth taking, and each one after it
	// is as big as everything taken so far.
	REQUIRE_CALL(fixture.get_source(), allocate(min_heap))
		.LR_RETURN(fixture.get_arena().take(_1))
		.IN_SEQUENCE(sequence);
	REQUIRE_CALL(fixture.get_source(), allocate(min_heap))
		.LR_RETURN(fixture.get_arena().take(_1))
		.IN_SEQUENCE(sequence);
	REQUIRE_CALL(fixture.get_source(), allocate(2 * min_heap))
		.LR_RETURN(fixture.get_arena().take(_1))
		.IN_SEQUENCE(sequence);

	// Held, so that each request has to be served out of new memory.
	held_blocks blocks(fixture.get());
	blocks.take(min_heap);
	blocks.take(min_heap);
	blocks.take(2 * min_heap);
}

TEST_CASE(
	"a memory_block_allocator should ask for less rather than give up",
	"[memory_block_allocator]"
)
{
	// Enough for the request, but nowhere near what growing by doubling would
	// like to take.
	cuda::block_allocator_fixture fixture(3 * min_heap);

	held_blocks blocks(fixture.get());
	blocks.take(2 * min_heap);

	trompeloeil::sequence sequence;

	// Asks for as much again as it already holds, is refused, and halves until
	// what the device has left is enough.
	REQUIRE_CALL(fixture.get_source(), allocate(2 * min_heap))
		.THROW(std::bad_alloc())
		.IN_SEQUENCE(sequence);
	REQUIRE_CALL(fixture.get_source(), allocate(min_heap))
		.LR_RETURN(fixture.get_arena().take(_1))
		.IN_SEQUENCE(sequence);

	CHECK( blocks.take(min_heap).get_size() >= min_heap );
}

TEST_CASE(
	"a memory_block_allocator should give memory back rather than fail",
	"[memory_block_allocator]"
)
{
	cuda::block_allocator_fixture fixture(2 * min_heap);

	// Taken and dropped, so the allocator is holding a heap it is not using.
	fixture.allocate_and_drop(min_heap, command_token());
	REQUIRE( fixture.get_arena().get_used_bytes() == min_heap );

	// More than what is left, but not more than the device has. The only way
	// through is to hand the idle heap back first.
	held_blocks blocks(fixture.get());
	CHECK( blocks.take(2 * min_heap).get_size() >= 2 * min_heap );
	CHECK( fixture.get_arena().get_region_count() == 1 );
}

TEST_CASE(
	"a memory_block_allocator should give up when the device really is full",
	"[memory_block_allocator]"
)
{
	cuda::block_allocator_fixture fixture(min_heap);

	// Held, so there is nothing idle to hand back.
	held_blocks blocks(fixture.get());
	blocks.take(min_heap);

	CHECK_THROWS_AS(
		fixture.get().allocate(min_heap, alignment, nullptr),
		std::bad_alloc
	);
}

TEST_CASE(
	"a memory_block_allocator should give a large request a heap of its own",
	"[memory_block_allocator]"
)
{
	cuda::block_allocator_fixture fixture;
	const std::size_t size =
		REXLIB_CUDA_CACHING_ALLOCATOR_LARGE_ALLOCATION_BYTES;

	// Sized to the request rather than rounded up, since a heap this big would
	// otherwise sit there holding memory nothing else is small enough to use.
	REQUIRE_CALL(fixture.get_source(), allocate(size))
		.LR_RETURN(fixture.get_arena().take(_1));

	held_blocks blocks(fixture.get());
	CHECK( blocks.take(size).get_size() == size );
}

TEST_CASE(
	"a memory_block_allocator should hand out a block that a command still "
	"uses rather than grow",
	"[memory_block_allocator]"
)
{
	cuda::block_allocator_fixture fixture;
	const auto &first = fixture.get_timeline(0);
	const auto &second = fixture.get_timeline(1);

	// Taken and dropped while a command of the first timeline still uses it.
	REQUIRE_CALL(fixture.get_source(), allocate(min_heap))
		.LR_RETURN(fixture.get_arena().take(_1));
	const auto *data =
		fixture.allocate_and_drop(min_heap, command_token(first, 4));

	// Whoever uses the block next waits for that command, through the token
	// the block carries. The allocator itself waits for nothing.
	FORBID_CALL(*first, wait(trompeloeil::_));

	held_blocks blocks(fixture.get());

	SECTION( "to a queue of another timeline" )
	{
		auto &block = blocks.take(min_heap, second.get());

		CHECK( block.get_data() == data );
		CHECK( block.get_pending().get_timeline() == first );
		CHECK( block.get_pending().get_id() == 4 );
	}

	SECTION( "and to an allocation that named no queue" )
	{
		auto &block = blocks.take(min_heap, nullptr);

		CHECK( block.get_data() == data );
		CHECK( block.get_pending().get_timeline() == first );
		CHECK( block.get_pending().get_id() == 4 );
	}
}

TEST_CASE(
	"a memory_block_allocator should prefer the block its queue can use at "
	"no cost",
	"[memory_block_allocator]"
)
{
	cuda::block_allocator_fixture fixture;
	auto &allocator = fixture.get();
	const command_token first(fixture.get_timeline(0), 1);
	const command_token second(fixture.get_timeline(1), 1);

	// Two halves of one heap, dropped while commands of different timelines
	// still use them.
	auto &first_block = allocator.allocate(min_heap / 2, alignment, nullptr);
	auto &second_block = allocator.allocate(min_heap / 2, alignment, nullptr);
	const auto *first_data = first_block.get_data();
	const auto *second_data = second_block.get_data();
	allocator.recycle(first_block, make_span(&first, 1));
	allocator.recycle(second_block, make_span(&second, 1));

	held_blocks blocks(fixture.get());

	SECTION( "the one of the first timeline" )
	{
		auto &block = blocks.take(min_heap / 2, first.get_timeline().get());
		CHECK( block.get_data() == first_data );
	}

	SECTION( "the one of the second timeline" )
	{
		auto &block = blocks.take(min_heap / 2, second.get_timeline().get());
		CHECK( block.get_data() == second_data );
	}
}

TEST_CASE(
	"a memory_block_allocator should hold back a block that commands of "
	"several timelines still use",
	"[memory_block_allocator]"
)
{
	cuda::block_allocator_fixture fixture;
	auto &allocator = fixture.get();
	const auto &first = fixture.get_timeline(0);
	const auto &second = fixture.get_timeline(1);

	auto &dropped = allocator.allocate(min_heap, alignment, nullptr);
	const auto *data = dropped.get_data();

	const std::array<command_token, 2> tokens = {
		command_token(first, 1),
		command_token(second, 1)
	};
	allocator.recycle(dropped, make_span(tokens));

	held_blocks blocks(fixture.get());

	SECTION( "while both are running" )
	{
		// A block carries a single token, so this one cannot be handed out,
		// and the request is served out of new memory.
		CHECK( blocks.take(min_heap).get_data() != data );
	}

	SECTION( "until one of them is left" )
	{
		ALLOW_CALL(*first, is_complete(1u))
			.RETURN(true);

		auto &block = blocks.take(min_heap);

		CHECK( block.get_data() == data );
		CHECK( block.get_pending().get_timeline() == second );
		CHECK( block.get_pending().get_id() == 1 );
	}
}

TEST_CASE(
	"trimming a memory_block_allocator should give back what is idle",
	"[memory_block_allocator]"
)
{
	cuda::block_allocator_fixture fixture;

	SECTION( "all of it when nothing is in use" )
	{
		fixture.allocate_and_drop(1024, command_token());
		REQUIRE( fixture.get_arena().get_used_bytes() == min_heap );

		CHECK( fixture.get().trim() == min_heap );
		CHECK( fixture.get_arena().get_region_count() == 0 );
	}

	SECTION( "and none of it while a block is out" )
	{
		held_blocks blocks(fixture.get());
		blocks.take(1024);

		CHECK( fixture.get().trim() == 0 );
		CHECK( fixture.get_arena().get_used_bytes() == min_heap );
	}
}

TEST_CASE(
	"trimming a memory_block_allocator should merge back what the timelines "
	"kept apart",
	"[memory_block_allocator]"
)
{
	cuda::block_allocator_fixture fixture;
	const auto &first = fixture.get_timeline(0);
	const auto &second = fixture.get_timeline(1);

	// Two halves of one heap, dropped while commands of different timelines
	// still use them, so neither can absorb the other.
	fixture.allocate_and_drop(min_heap / 2, command_token(first, 1));
	fixture.allocate_and_drop(min_heap / 2, command_token(second, 1));
	REQUIRE( fixture.get_arena().get_used_bytes() == min_heap );

	// Waiting for both commands leaves nothing to wait for, and the halves
	// add back up to the heap they came from.
	REQUIRE_CALL(*first, wait(1u));
	REQUIRE_CALL(*second, wait(1u));

	CHECK( fixture.get().trim() == min_heap );
	CHECK( fixture.get_arena().get_region_count() == 0 );
}

TEST_CASE(
	"a memory_block_allocator should wait for what still uses its memory "
	"before giving it back to the driver",
	"[memory_block_allocator]"
)
{
	// Declared before the allocator, so that it is still there to be asked
	// when the allocator dies.
	const auto timeline = std::make_shared<cuda::mock_command_timeline>();
	REQUIRE_CALL(*timeline, wait(3u));

	cuda::block_allocator_fixture fixture;
	fixture.allocate_and_drop(1024, command_token(timeline, 3));
}
