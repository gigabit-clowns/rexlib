// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <hardware/caching_allocator/memory_block_pool.hpp>

#include <hardware/caching_allocator/memory_heap.hpp>

#include "../mock/mock_command_timeline.hpp"
#include "mock/mock_memory_source.hpp"

#include <rexlib/core/hardware/command_token.hpp>

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace rexlib;

namespace
{

/// Stands in for whatever the driver would have handed out. Big enough that
/// every heap a test asks for gets a region of its own.
alignas(256) std::byte storage[1 << 20];

/**
 * @brief A pool over heaps that exist only as addresses.
 *
 * Hands each heap a region of its own out of one static block, so that the
 * pool sees distinct heaps that never overlap, without a device to take them
 * from.
 */
class pool_fixture
{
public:
	pool_fixture()
		: m_used(0)
		, m_alignment_expectation(
			NAMED_ALLOW_CALL(m_source, get_base_alignment())
				.RETURN(256)
		)
		, m_allocate_expectation(
			NAMED_ALLOW_CALL(m_source, allocate(trompeloeil::_))
				.LR_RETURN(this->take(_1))
		)
		, m_deallocate_expectation(
			NAMED_ALLOW_CALL(m_source, deallocate(trompeloeil::_))
		)
	{
	}

	cuda::memory_block_pool& get() noexcept
	{
		return m_pool;
	}

	/**
	 * @brief Take a heap of the given size into the pool.
	 *
	 * @param size Size of the heap, in bytes.
	 * @return cuda::memory_block* The block spanning it.
	 */
	cuda::memory_block* add_heap(std::size_t size)
	{
		return m_pool.register_heap(
			std::make_unique<cuda::memory_heap>(m_source, size, 256)
		);
	}

private:
	cuda::mock_memory_source m_source;
	std::size_t m_used;

	// The pool gives its heaps back when it dies, so what says that is
	// expected has to still be around by then. Declared before the pool, so
	// that they are destroyed after it.
	std::unique_ptr<trompeloeil::expectation> m_alignment_expectation;
	std::unique_ptr<trompeloeil::expectation> m_allocate_expectation;
	std::unique_ptr<trompeloeil::expectation> m_deallocate_expectation;

	cuda::memory_block_pool m_pool;

	void* take(std::size_t size)
	{
		// Handing out what is not there would quietly corrupt whatever sits
		// after the storage, and read as the pool misbehaving.
		REQUIRE( m_used + size <= sizeof(storage) );

		auto *result = storage + m_used;
		m_used += size;
		return result;
	}
};

/// Cut a heap into equally sized blocks, left to right.
std::vector<cuda::memory_block*> partition_evenly(
	cuda::memory_block_pool &pool,
	cuda::memory_block &block,
	std::size_t count
)
{
	std::vector<cuda::memory_block*> result;

	auto *rest = &block;
	const auto part = block.get_size() / count;
	for (std::size_t i = 1; i < count; ++i)
	{
		const auto halves = pool.partition_block(
			*rest,
			part,
			rest->get_size() - part
		);
		result.push_back(halves.first);
		rest = halves.second;
	}
	result.push_back(rest);

	return result;
}

/// Hand a free block out and take it straight back carrying a token, which
/// is what a buffer dropped while a command still uses it does.
void give_back_with(
	cuda::memory_block_pool &pool,
	cuda::memory_block &block,
	command_token pending
)
{
	pool.acquire(block);
	pool.release(block, std::move(pending));
}

} // anonymous namespace

TEST_CASE(
	"a heap taken into a memory_block_pool should arrive as one free block "
	"carrying an empty token",
	"[memory_block_pool]"
)
{
	pool_fixture fixture;

	auto *block = fixture.add_heap(4096);

	REQUIRE( block != nullptr );
	CHECK( block->get_size() == 4096 );
	CHECK( block->get_offset() == 0 );
	CHECK( block->is_free() );

	// Nothing has ever run against memory the driver has only just handed
	// over, so any queue can take it without waiting.
	CHECK( block->get_pending().is_empty() );

	CHECK( fixture.get().get_size() == 4096 );
	CHECK( fixture.get().get_acquired_block_count() == 0 );
}

TEST_CASE(
	"memory_block_pool should refuse a null heap",
	"[memory_block_pool]"
)
{
	pool_fixture fixture;

	CHECK_THROWS_AS(
		fixture.get().register_heap(nullptr),
		std::invalid_argument
	);
}

TEST_CASE(
	"memory_block_pool should hand out the smallest block that fits",
	"[memory_block_pool]"
)
{
	pool_fixture fixture;
	auto &pool = fixture.get();
	const auto timeline = std::make_shared<cuda::mock_command_timeline>();

	// One heap each, so that carrying tokens of one timeline cannot turn the
	// four candidates back into one.
	for (const std::size_t size : {256, 512, 768, 1024})
	{
		give_back_with(
			pool, *fixture.add_heap(size), command_token(timeline, 1)
		);
	}

	SECTION( "when one is an exact fit" )
	{
		auto *found = pool.find_suitable_block(512, timeline.get());
		REQUIRE( found != nullptr );
		CHECK( found->get_size() == 512 );
	}

	SECTION( "when none is an exact fit" )
	{
		auto *found = pool.find_suitable_block(600, timeline.get());
		REQUIRE( found != nullptr );
		CHECK( found->get_size() == 768 );
	}

	SECTION( "and nothing when none is big enough" )
	{
		CHECK( pool.find_suitable_block(4096, timeline.get()) == nullptr );
	}
}

TEST_CASE(
	"memory_block_pool should only hand a queue what it can use at no cost",
	"[memory_block_pool]"
)
{
	pool_fixture fixture;
	auto &pool = fixture.get();
	const auto own = std::make_shared<cuda::mock_command_timeline>();
	const auto other = std::make_shared<cuda::mock_command_timeline>();

	// One block per timeline, and one that nothing uses.
	auto *own_block = fixture.add_heap(1024);
	auto *unused_block = fixture.add_heap(1024);
	auto *other_block = fixture.add_heap(1024);
	give_back_with(pool, *own_block, command_token(own, 1));
	give_back_with(pool, *other_block, command_token(other, 1));

	SECTION( "blocks of its own timeline, which it waits for by running" )
	{
		auto *found = pool.find_suitable_block(1024, own.get());
		REQUIRE( found != nullptr );
		CHECK( found->get_pending().get_timeline() == own );
	}

	SECTION( "and blocks with an empty token, which nothing runs against" )
	{
		pool.acquire(*own_block);

		auto *found = pool.find_suitable_block(1024, own.get());
		REQUIRE( found != nullptr );
		CHECK( found->get_pending().is_empty() );

		pool.release(*own_block, command_token(own, 1));
	}

	SECTION( "but never one of another timeline, which may still be in use" )
	{
		pool.acquire(*own_block);
		pool.acquire(*unused_block);

		CHECK( pool.find_suitable_block(1024, own.get()) == nullptr );

		pool.release(*own_block, command_token(own, 1));
		pool.release(*unused_block, command_token());
	}

	SECTION( "and only blocks with an empty token when it names no timeline" )
	{
		auto *found = pool.find_suitable_block(1024, nullptr);
		REQUIRE( found != nullptr );
		CHECK( found == unused_block );
	}
}

TEST_CASE(
	"memory_block_pool should offer the smallest suitable block of another "
	"timeline",
	"[memory_block_pool]"
)
{
	pool_fixture fixture;
	auto &pool = fixture.get();
	const auto own = std::make_shared<cuda::mock_command_timeline>();
	const auto second = std::make_shared<cuda::mock_command_timeline>();
	const auto third = std::make_shared<cuda::mock_command_timeline>();

	// One heap each, so that neighbouring blocks cannot merge into one.
	give_back_with(pool, *fixture.add_heap(512), command_token(own, 1));
	give_back_with(pool, *fixture.add_heap(1024), command_token(second, 1));
	give_back_with(pool, *fixture.add_heap(2048), command_token(third, 1));
	fixture.add_heap(512); // Left with an empty token.

	SECTION( "picking the tightest fit across every other timeline" )
	{
		auto *found = pool.find_foreign_block(768, own.get());
		REQUIRE( found != nullptr );
		CHECK( found->get_pending().get_timeline() == second );
		CHECK( found->get_size() == 1024 );
	}

	SECTION( "never one of its own, which costs it nothing" )
	{
		auto *found = pool.find_foreign_block(256, own.get());
		REQUIRE( found != nullptr );
		CHECK( found->get_pending().get_timeline() != own );
	}

	SECTION( "never one with an empty token, which costs it nothing" )
	{
		auto *found = pool.find_foreign_block(512, own.get());
		REQUIRE( found != nullptr );
		CHECK_FALSE( found->get_pending().is_empty() );
	}

	SECTION( "one of any timeline when it names none" )
	{
		auto *found = pool.find_foreign_block(256, nullptr);
		REQUIRE( found != nullptr );
		CHECK( found->get_pending().get_timeline() == own );
	}

	SECTION( "and nothing when none is big enough" )
	{
		CHECK( pool.find_foreign_block(4096, own.get()) == nullptr );
	}
}

TEST_CASE(
	"a block acquired from a memory_block_pool should keep its token until "
	"it is given back",
	"[memory_block_pool]"
)
{
	pool_fixture fixture;
	auto &pool = fixture.get();
	const auto timeline = std::make_shared<cuda::mock_command_timeline>();

	auto *block = fixture.add_heap(1024);
	give_back_with(pool, *block, command_token(timeline, 4));
	pool.acquire(*block);

	// The token is what whoever was handed the block has to wait for.
	CHECK_FALSE( block->is_free() );
	CHECK( block->get_pending().get_timeline() == timeline );
	CHECK( block->get_pending().get_id() == 4 );
	CHECK( pool.find_suitable_block(1, timeline.get()) == nullptr );
	CHECK( pool.get_acquired_block_count() == 1 );

	pool.release(*block, command_token(timeline, 9));

	CHECK( block->is_free() );
	CHECK( block->get_pending().get_id() == 9 );
	CHECK( pool.get_acquired_block_count() == 0 );
}

TEST_CASE(
	"partitioning a block of a memory_block_pool should leave two free halves",
	"[memory_block_pool]"
)
{
	pool_fixture fixture;
	auto &pool = fixture.get();
	const auto timeline = std::make_shared<cuda::mock_command_timeline>();

	auto *block = fixture.add_heap(1024);
	give_back_with(pool, *block, command_token(timeline, 4));

	const auto halves = pool.partition_block(*block, 256, 768);

	REQUIRE( halves.first != nullptr );
	REQUIRE( halves.second != nullptr );
	CHECK( halves.first != halves.second );

	CHECK( halves.first->get_offset() == 0 );
	CHECK( halves.first->get_size() == 256 );
	CHECK( halves.first->is_free() );
	CHECK( halves.first->get_pending().get_timeline() == timeline );
	CHECK( halves.first->get_pending().get_id() == 4 );

	CHECK( halves.second->get_offset() == 256 );
	CHECK( halves.second->get_size() == 768 );
	CHECK( halves.second->is_free() );
	CHECK( halves.second->get_pending().get_timeline() == timeline );
	CHECK( halves.second->get_pending().get_id() == 4 );

	CHECK( halves.first->get_heap() == halves.second->get_heap() );
}

TEST_CASE(
	"releasing a block to a memory_block_pool should merge it with its free "
	"neighbours",
	"[memory_block_pool]"
)
{
	pool_fixture fixture;
	auto &pool = fixture.get();
	const auto timeline = std::make_shared<cuda::mock_command_timeline>();
	const command_token token(timeline, 1);

	auto blocks = partition_evenly(pool, *fixture.add_heap(1024), 4);
	for (auto *block : blocks)
	{
		pool.acquire(*block);
	}

	SECTION( "not at all when both neighbours are still in use" )
	{
		pool.release(*blocks[1], token);

		CHECK( blocks[1]->get_size() == 256 );
		CHECK( blocks[1]->get_offset() == 256 );

		pool.release(*blocks[0], token);
		pool.release(*blocks[2], token);
		pool.release(*blocks[3], token);
	}

	SECTION( "forwards" )
	{
		pool.release(*blocks[2], token);
		pool.release(*blocks[1], token);

		CHECK( blocks[1]->get_size() == 512 );
		CHECK( blocks[1]->get_offset() == 256 );

		pool.release(*blocks[0], token);
		pool.release(*blocks[3], token);
	}

	SECTION( "backwards" )
	{
		pool.release(*blocks[0], token);
		pool.release(*blocks[1], token);

		CHECK( blocks[1]->get_size() == 512 );
		CHECK( blocks[1]->get_offset() == 0 );

		pool.release(*blocks[2], token);
		pool.release(*blocks[3], token);
	}

	SECTION( "both ways at once" )
	{
		pool.release(*blocks[0], token);
		pool.release(*blocks[2], token);
		pool.release(*blocks[3], token);
		pool.release(*blocks[1], token);

		CHECK( blocks[1]->get_size() == 1024 );
		CHECK( blocks[1]->get_offset() == 0 );
	}
}

TEST_CASE(
	"blocks merged by a memory_block_pool should carry the later of their "
	"tokens",
	"[memory_block_pool]"
)
{
	pool_fixture fixture;
	auto &pool = fixture.get();
	const auto timeline = std::make_shared<cuda::mock_command_timeline>();

	const auto blocks = partition_evenly(pool, *fixture.add_heap(1024), 2);
	pool.acquire(*blocks[0]);
	pool.acquire(*blocks[1]);

	// A stream finishes its commands in order, so waiting for the later one
	// is waiting for both.
	SECTION( "when the later one comes back last" )
	{
		pool.release(*blocks[0], command_token(timeline, 3));
		pool.release(*blocks[1], command_token(timeline, 7));

		CHECK( blocks[1]->get_size() == 1024 );
		CHECK( blocks[1]->get_pending().get_id() == 7 );
	}

	SECTION( "when the later one comes back first" )
	{
		pool.release(*blocks[1], command_token(timeline, 7));
		pool.release(*blocks[0], command_token(timeline, 3));

		CHECK( blocks[0]->get_size() == 1024 );
		CHECK( blocks[0]->get_pending().get_id() == 7 );
	}

	SECTION( "and none when neither has one" )
	{
		pool.release(*blocks[0], command_token());
		pool.release(*blocks[1], command_token());

		CHECK( blocks[1]->get_size() == 1024 );
		CHECK( blocks[1]->get_pending().is_empty() );
	}
}

TEST_CASE(
	"a memory_block_pool should not merge blocks that a single token could "
	"not speak for",
	"[memory_block_pool]"
)
{
	pool_fixture fixture;
	auto &pool = fixture.get();
	const auto first = std::make_shared<cuda::mock_command_timeline>();
	const auto second = std::make_shared<cuda::mock_command_timeline>();

	const auto blocks = partition_evenly(pool, *fixture.add_heap(1024), 2);
	pool.acquire(*blocks[0]);
	pool.acquire(*blocks[1]);

	// Neighbours in one heap, but what has to have finished before each can
	// be used is different, so one block out of the two would be a lie.
	SECTION( "when their tokens are of different timelines" )
	{
		pool.release(*blocks[1], command_token(second, 1));
		pool.release(*blocks[0], command_token(first, 1));
	}

	SECTION( "when only one of them is still in use" )
	{
		pool.release(*blocks[1], command_token());
		pool.release(*blocks[0], command_token(first, 1));
	}

	CHECK( blocks[0]->get_size() == 512 );
	CHECK( blocks[1]->get_size() == 512 );
}

TEST_CASE(
	"a memory_block_pool should not merge blocks of different heaps",
	"[memory_block_pool]"
)
{
	pool_fixture fixture;
	auto &pool = fixture.get();

	auto *first = fixture.add_heap(512);
	auto *second = fixture.add_heap(512);

	pool.acquire(*first);
	pool.acquire(*second);
	pool.release(*second, command_token());
	pool.release(*first, command_token());

	// Two heaps are two calls to the driver and have to be given back as
	// such, whatever the addresses happen to look like.
	CHECK( first->get_size() == 512 );
	CHECK( second->get_size() == 512 );
}

TEST_CASE(
	"emptying the tokens of a memory_block_pool should merge what the "
	"timelines kept apart",
	"[memory_block_pool]"
)
{
	pool_fixture fixture;
	auto &pool = fixture.get();
	const auto first = std::make_shared<cuda::mock_command_timeline>();
	const auto second = std::make_shared<cuda::mock_command_timeline>();

	const auto blocks = partition_evenly(pool, *fixture.add_heap(1024), 4);
	give_back_with(pool, *blocks[0], command_token(first, 1));
	give_back_with(pool, *blocks[1], command_token(second, 1));
	give_back_with(pool, *blocks[2], command_token(first, 2));
	give_back_with(pool, *blocks[3], command_token(second, 2));

	pool.reset_pending();

	// Once nothing is running any more, the four blocks are just the heap
	// again, and any queue can have it.
	auto *found = pool.find_suitable_block(1024, first.get());
	REQUIRE( found != nullptr );
	CHECK( found->get_size() == 1024 );
	CHECK( found->get_offset() == 0 );
	CHECK( found->get_pending().is_empty() );
}

TEST_CASE(
	"emptying the tokens of a memory_block_pool should leave the blocks in "
	"use alone",
	"[memory_block_pool]"
)
{
	pool_fixture fixture;
	auto &pool = fixture.get();
	const auto timeline = std::make_shared<cuda::mock_command_timeline>();

	auto *heap = fixture.add_heap(1024);
	give_back_with(pool, *heap, command_token(timeline, 2));

	const auto blocks = partition_evenly(pool, *heap, 4);
	for (auto *block : blocks)
	{
		pool.acquire(*block);
	}
	pool.release(*blocks[0], command_token(timeline, 5));
	pool.release(*blocks[3], command_token(timeline, 5));

	pool.reset_pending();

	CHECK( pool.get_acquired_block_count() == 2 );
	CHECK( blocks[1]->get_pending().get_timeline() == timeline );
	CHECK( blocks[2]->get_pending().get_timeline() == timeline );
	CHECK( blocks[0]->get_pending().is_empty() );
	CHECK( blocks[3]->get_pending().is_empty() );
	CHECK( blocks[0]->get_size() == 256 );
	CHECK( blocks[3]->get_size() == 256 );

	pool.release(*blocks[1], command_token());
	pool.release(*blocks[2], command_token());
}

TEST_CASE(
	"a memory_block_pool should give back the heaps it is no longer using",
	"[memory_block_pool]"
)
{
	pool_fixture fixture;
	auto &pool = fixture.get();

	SECTION( "when a heap is one whole free block" )
	{
		fixture.add_heap(1024);

		CHECK( pool.release_unused_heaps() == 1024 );
		CHECK( pool.get_size() == 0 );
		CHECK( pool.find_suitable_block(1, nullptr) == nullptr );
	}

	SECTION( "but not while any of it is in use" )
	{
		auto *block = fixture.add_heap(1024);
		pool.acquire(*block);

		CHECK( pool.release_unused_heaps() == 0 );
		CHECK( pool.get_size() == 1024 );

		pool.release(*block, command_token());
	}

	SECTION( "but not while it is still cut up" )
	{
		const auto blocks = partition_evenly(pool, *fixture.add_heap(1024), 2);
		pool.acquire(*blocks[0]);

		CHECK( pool.release_unused_heaps() == 0 );
		CHECK( pool.get_size() == 1024 );

		pool.release(*blocks[0], command_token());
	}

	SECTION( "and only the ones that are done with" )
	{
		auto *kept = fixture.add_heap(1024);
		fixture.add_heap(512);
		pool.acquire(*kept);

		CHECK( pool.release_unused_heaps() == 512 );
		CHECK( pool.get_size() == 1024 );

		pool.release(*kept, command_token());
	}
}

TEST_CASE(
	"a memory_block_pool should report what its free blocks are still used "
	"by",
	"[memory_block_pool]"
)
{
	pool_fixture fixture;
	auto &pool = fixture.get();
	const auto first = std::make_shared<cuda::mock_command_timeline>();
	const auto second = std::make_shared<cuda::mock_command_timeline>();

	// One heap each, so that the two blocks of the first timeline stay two.
	give_back_with(pool, *fixture.add_heap(256), command_token(first, 8));
	give_back_with(pool, *fixture.add_heap(512), command_token(first, 5));
	give_back_with(pool, *fixture.add_heap(256), command_token(second, 2));
	fixture.add_heap(256); // Left with an empty token.

	std::vector<command_token> pending;
	pool.enumerate_pending(pending);

	// One token per timeline, the latest, since it speaks for the others. A
	// block that nothing uses has nothing to report.
	REQUIRE( pending.size() == 2 );
	for (const auto &token : pending)
	{
		if (token.get_timeline() == first)
		{
			CHECK( token.get_id() == 8 );
		}
		else
		{
			CHECK( token.get_timeline() == second );
			CHECK( token.get_id() == 2 );
		}
	}
	CHECK( pending[0].get_timeline() != pending[1].get_timeline() );
}
