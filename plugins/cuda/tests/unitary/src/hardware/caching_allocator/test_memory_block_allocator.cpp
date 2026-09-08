// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <hardware/caching_allocator/memory_block_allocator.hpp>

#include <hardware/caching_allocator/memory_block.hpp>

#include "mock/mock_event_recorder.hpp"
#include "mock/mock_memory_source.hpp"
#include "test_block_allocator.hpp"
#include "test_event_recorder_reference.hpp"
#include "test_memory_source_reference.hpp"
#include "test_queue.hpp"

#include <config.hpp>

#include <rexlib/core/memory/align.hpp>
#include <rexlib/core/span.hpp>

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
			m_allocator->recycle(*block, span<const cuda::queue_handle>());
		}
	}

	held_blocks& operator=(const held_blocks &other) = delete;
	held_blocks& operator=(held_blocks &&other) = delete;

	cuda::memory_block& take(
		std::size_t size,
		const cuda::queue_handle &queue
	)
	{
		auto &result = m_allocator->allocate(size, alignment, queue);
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

} // namespace

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
	"memory_block_allocator should refuse to be built without what it needs",
	"[memory_block_allocator]"
)
{
	cuda::mock_memory_source source;
	cuda::mock_event_recorder recorder;

	SECTION( "when it is given no memory source" )
	{
		CHECK_THROWS_AS(
			cuda::memory_block_allocator(
				nullptr,
				std::make_unique<cuda::event_recorder_reference>(recorder)
			),
			std::invalid_argument
		);
	}

	SECTION( "when it is given no event recorder" )
	{
		CHECK_THROWS_AS(
			cuda::memory_block_allocator(
				std::make_unique<cuda::memory_source_reference>(source),
				nullptr
			),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"memory_block_allocator should refuse an alignment it cannot promise",
	"[memory_block_allocator]"
)
{
	cuda::block_allocator_fixture fixture;
	const auto queue = cuda::make_test_queue(0);

	FORBID_CALL(fixture.get_source(), allocate(trompeloeil::_));

	SECTION( "when it is stricter than the maximum" )
	{
		CHECK_THROWS_AS(
			fixture.get().allocate(1024, 2 * alignment, queue),
			std::invalid_argument
		);
	}

	SECTION( "when it is not a power of two" )
	{
		CHECK_THROWS_AS(
			fixture.get().allocate(1024, 24, queue),
			std::invalid_argument
		);
	}

	SECTION( "when it is zero" )
	{
		CHECK_THROWS_AS(
			fixture.get().allocate(1024, 0, queue),
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

	auto &block = blocks.take(1000, cuda::make_test_queue(0));

	CHECK( is_aligned(block.get_data(), alignment) );

	// Rounding up is what keeps the remainder of a block usable, so the size
	// it reports is the rounded one rather than what was asked for.
	CHECK( block.get_size() == align_ceil(std::size_t(1000), alignment) );
}

TEST_CASE(
	"a memory_block_allocator should stop reaching the driver once it has "
	"memory of its own",
	"[memory_block_allocator]"
)
{
	cuda::block_allocator_fixture fixture;
	const auto queue = cuda::make_test_queue(0);

	REQUIRE_CALL(fixture.get_source(), allocate(min_heap))
		.LR_RETURN(fixture.get_arena().take(_1));

	const auto *first = fixture.allocate_and_drop(1024, queue);
	for (std::size_t i = 0; i < 1000; ++i)
	{
		// Handed straight back to the queue it came from, whose own work it is
		// already ordered against, so it lands in the same place every time.
		CHECK( fixture.allocate_and_drop(1024, queue) == first );
	}
}

TEST_CASE(
	"a memory_block_allocator should cut one heap into the blocks that are "
	"alive at once",
	"[memory_block_allocator]"
)
{
	cuda::block_allocator_fixture fixture;
	const auto queue = cuda::make_test_queue(0);

	REQUIRE_CALL(fixture.get_source(), allocate(min_heap))
		.LR_RETURN(fixture.get_arena().take(_1));

	held_blocks blocks(fixture.get());
	for (std::size_t i = 0; i < 4; ++i)
	{
		blocks.take(1024, queue);
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
	const auto queue = cuda::make_test_queue(0);

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
	blocks.take(min_heap, queue);
	blocks.take(min_heap, queue);
	blocks.take(2 * min_heap, queue);
}

TEST_CASE(
	"a memory_block_allocator should ask for less rather than give up",
	"[memory_block_allocator]"
)
{
	// Enough for the request, but nowhere near what growing by doubling would
	// like to take.
	cuda::block_allocator_fixture fixture(3 * min_heap);
	const auto queue = cuda::make_test_queue(0);

	held_blocks blocks(fixture.get());
	blocks.take(2 * min_heap, queue);

	trompeloeil::sequence sequence;

	// Asks for as much again as it already holds, is refused, and halves until
	// what the device has left is enough.
	REQUIRE_CALL(fixture.get_source(), allocate(2 * min_heap))
		.THROW(std::bad_alloc())
		.IN_SEQUENCE(sequence);
	REQUIRE_CALL(fixture.get_source(), allocate(min_heap))
		.LR_RETURN(fixture.get_arena().take(_1))
		.IN_SEQUENCE(sequence);

	CHECK( blocks.take(min_heap, queue).get_size() >= min_heap );
}

TEST_CASE(
	"a memory_block_allocator should give memory back rather than fail",
	"[memory_block_allocator]"
)
{
	cuda::block_allocator_fixture fixture(2 * min_heap);
	const auto queue = cuda::make_test_queue(0);

	// Taken and dropped, so the allocator is holding a heap it is not using.
	fixture.allocate_and_drop(min_heap, queue);
	REQUIRE( fixture.get_arena().get_used_bytes() == min_heap );

	// More than what is left, but not more than the device has. The only way
	// through is to hand the idle heap back first.
	held_blocks blocks(fixture.get());
	CHECK( blocks.take(2 * min_heap, queue).get_size() >= 2 * min_heap );
	CHECK( fixture.get_arena().get_region_count() == 1 );
}

TEST_CASE(
	"a memory_block_allocator should give up when the device really is full",
	"[memory_block_allocator]"
)
{
	cuda::block_allocator_fixture fixture(min_heap);
	const auto queue = cuda::make_test_queue(0);

	// Held, so there is nothing idle to hand back.
	held_blocks blocks(fixture.get());
	blocks.take(min_heap, queue);

	CHECK_THROWS_AS(
		fixture.get().allocate(min_heap, alignment, queue),
		std::bad_alloc
	);
}

TEST_CASE(
	"a memory_block_allocator should give a large request a heap of its own",
	"[memory_block_allocator]"
)
{
	cuda::block_allocator_fixture fixture;
	const auto queue = cuda::make_test_queue(0);
	const std::size_t size =
		REXLIB_CUDA_CACHING_ALLOCATOR_LARGE_ALLOCATION_BYTES;

	// Sized to the request rather than rounded up, since a heap this big would
	// otherwise sit there holding memory nothing else is small enough to use.
	REQUIRE_CALL(fixture.get_source(), allocate(size))
		.LR_RETURN(fixture.get_arena().take(_1));

	held_blocks blocks(fixture.get());
	CHECK( blocks.take(size, queue).get_size() == size );
}

TEST_CASE(
	"a memory_block_allocator should take a block over from another queue "
	"rather than grow",
	"[memory_block_allocator]"
)
{
	cuda::block_allocator_fixture fixture;
	const auto first = cuda::make_test_queue(0);
	const auto second = cuda::make_test_queue(1);

	// Taken by one queue and dropped, so it belongs to that queue's timeline.
	REQUIRE_CALL(fixture.get_source(), allocate(min_heap))
		.LR_RETURN(fixture.get_arena().take(_1));
	const auto *data = fixture.allocate_and_drop(min_heap, first);

	// Reusing it costs a wait on the device rather than a call to the driver,
	// which is the whole point of knowing which queue a block belongs to.
	REQUIRE_CALL(fixture.get_recorder(), record(first))
		.LR_RETURN(1);
	REQUIRE_CALL(fixture.get_recorder(), enqueue_wait(second, 1u));
	FORBID_CALL(fixture.get_recorder(), wait(trompeloeil::_));

	held_blocks blocks(fixture.get());
	CHECK( blocks.take(min_heap, second).get_data() == data );
}

TEST_CASE(
	"a memory_block_allocator should not take a block over for an allocation "
	"that named no queue",
	"[memory_block_allocator]"
)
{
	cuda::block_allocator_fixture fixture;

	fixture.allocate_and_drop(min_heap, cuda::make_test_queue(0));

	// There is nowhere to put the wait that taking the block over would need,
	// so the only way to serve this is out of memory nothing is running
	// against.
	REQUIRE_CALL(fixture.get_source(), allocate(trompeloeil::_))
		.LR_RETURN(fixture.get_arena().take(_1));

	held_blocks blocks(fixture.get());
	blocks.take(min_heap, cuda::queue_handle());
}

TEST_CASE(
	"trimming a memory_block_allocator should give back what is idle",
	"[memory_block_allocator]"
)
{
	cuda::block_allocator_fixture fixture;
	const auto queue = cuda::make_test_queue(0);

	SECTION( "all of it when nothing is in use" )
	{
		fixture.allocate_and_drop(1024, queue);
		REQUIRE( fixture.get_arena().get_used_bytes() == min_heap );

		CHECK( fixture.get().trim() == min_heap );
		CHECK( fixture.get_arena().get_region_count() == 0 );
	}

	SECTION( "and none of it while a block is out" )
	{
		held_blocks blocks(fixture.get());
		blocks.take(1024, queue);

		CHECK( fixture.get().trim() == 0 );
		CHECK( fixture.get_arena().get_used_bytes() == min_heap );
	}
}

TEST_CASE(
	"trimming a memory_block_allocator should merge back what the queues kept "
	"apart",
	"[memory_block_allocator]"
)
{
	cuda::block_allocator_fixture fixture;

	// Two halves of one heap, dropped while belonging to different queues, so
	// neither can absorb the other.
	fixture.allocate_and_drop(min_heap / 2, cuda::make_test_queue(0));
	fixture.allocate_and_drop(min_heap / 2, cuda::make_test_queue(1));
	REQUIRE( fixture.get_arena().get_used_bytes() == min_heap );

	// Waiting for both queues makes the two answers to "when is this safe" the
	// same one, and the halves add back up to the heap they came from.
	REQUIRE_CALL(fixture.get_recorder(), wait(trompeloeil::_))
		.TIMES(2);

	CHECK( fixture.get().trim() == min_heap );
	CHECK( fixture.get_arena().get_region_count() == 0 );
}
