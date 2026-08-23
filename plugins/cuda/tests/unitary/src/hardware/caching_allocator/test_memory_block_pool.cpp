// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <hardware/caching_allocator/memory_block_pool.hpp>

#include <hardware/caching_allocator/memory_heap.hpp>

#include "mock/mock_memory_source.hpp"
#include "test_queue.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace xmipp4;

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

} // namespace

TEST_CASE(
	"a heap taken into a memory_block_pool should arrive as one free block "
	"belonging to no queue",
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
	CHECK_FALSE( static_cast<bool>(block->get_queue()) );

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
	const auto queue = cuda::make_test_queue(0);

	// One heap each, so that binding them to a queue cannot turn the four
	// candidates back into one.
	for (const std::size_t size : {256, 512, 768, 1024})
	{
		auto *block = fixture.add_heap(size);
		pool.acquire(*block, queue);
		pool.release(*block);
	}

	SECTION( "when one is an exact fit" )
	{
		auto *found = pool.find_suitable_block(512, queue);
		REQUIRE( found != nullptr );
		CHECK( found->get_size() == 512 );
	}

	SECTION( "when none is an exact fit" )
	{
		auto *found = pool.find_suitable_block(600, queue);
		REQUIRE( found != nullptr );
		CHECK( found->get_size() == 768 );
	}

	SECTION( "and nothing when none is big enough" )
	{
		CHECK( pool.find_suitable_block(4096, queue) == nullptr );
	}
}

TEST_CASE(
	"memory_block_pool should only hand a queue what it can use without "
	"waiting",
	"[memory_block_pool]"
)
{
	pool_fixture fixture;
	auto &pool = fixture.get();
	const auto own = cuda::make_test_queue(0);
	const auto other = cuda::make_test_queue(1);

	// One block per queue, and one still belonging to none.
	auto *own_block = fixture.add_heap(1024);
	auto *unbound_block = fixture.add_heap(1024);
	auto *other_block = fixture.add_heap(1024);
	pool.acquire(*own_block, own);
	pool.release(*own_block);
	pool.acquire(*other_block, other);
	pool.release(*other_block);

	SECTION( "its own blocks, whose work it is already ordered against" )
	{
		auto *found = pool.find_suitable_block(1024, own);
		REQUIRE( found != nullptr );
		CHECK( found->get_queue() == own );
	}

	SECTION( "and unbound blocks, which nothing is running against" )
	{
		pool.acquire(*own_block, own);

		auto *found = pool.find_suitable_block(1024, own);
		REQUIRE( found != nullptr );
		CHECK_FALSE( static_cast<bool>(found->get_queue()) );

		pool.release(*own_block);
	}

	SECTION( "but never another queue's, which may still be in use" )
	{
		pool.acquire(*own_block, own);
		pool.acquire(*unbound_block, own);

		CHECK( pool.find_suitable_block(1024, own) == nullptr );

		pool.release(*own_block);
		pool.release(*unbound_block);
	}
}

TEST_CASE(
	"memory_block_pool should offer another queue's smallest suitable block",
	"[memory_block_pool]"
)
{
	pool_fixture fixture;
	auto &pool = fixture.get();
	const auto own = cuda::make_test_queue(0);
	const auto second = cuda::make_test_queue(1);
	const auto third = cuda::make_test_queue(2);

	// One heap each, so that neighbouring blocks cannot merge into one.
	auto *own_block = fixture.add_heap(512);
	auto *second_block = fixture.add_heap(1024);
	auto *third_block = fixture.add_heap(2048);
	fixture.add_heap(512); // Left belonging to no queue.

	pool.acquire(*own_block, own);
	pool.release(*own_block);
	pool.acquire(*second_block, second);
	pool.release(*second_block);
	pool.acquire(*third_block, third);
	pool.release(*third_block);

	SECTION( "picking the tightest fit across every other queue" )
	{
		auto *found = pool.find_foreign_block(768, own);
		REQUIRE( found != nullptr );
		CHECK( found->get_queue() == second );
		CHECK( found->get_size() == 1024 );
	}

	SECTION( "never its own, which it would not have to wait for" )
	{
		auto *found = pool.find_foreign_block(256, own);
		REQUIRE( found != nullptr );
		CHECK( found->get_queue() != own );
	}

	SECTION( "never an unbound one, which it would not have to wait for" )
	{
		auto *found = pool.find_foreign_block(512, own);
		REQUIRE( found != nullptr );
		CHECK( static_cast<bool>(found->get_queue()) );
	}

	SECTION( "and nothing when none is big enough" )
	{
		CHECK( pool.find_foreign_block(4096, own) == nullptr );
	}
}

TEST_CASE(
	"acquiring a block from a memory_block_pool should bind it to the queue "
	"that took it",
	"[memory_block_pool]"
)
{
	pool_fixture fixture;
	auto &pool = fixture.get();
	const auto queue = cuda::make_test_queue(0);

	auto *block = fixture.add_heap(1024);
	pool.acquire(*block, queue);

	CHECK_FALSE( block->is_free() );
	CHECK( block->get_queue() == queue );
	CHECK( pool.find_suitable_block(1, queue) == nullptr );
	CHECK( pool.get_acquired_block_count() == 1 );

	pool.release(*block);

	CHECK( block->is_free() );
	CHECK( block->get_queue() == queue );
	CHECK( pool.get_acquired_block_count() == 0 );
}

TEST_CASE(
	"partitioning a block of a memory_block_pool should leave two free halves",
	"[memory_block_pool]"
)
{
	pool_fixture fixture;
	auto &pool = fixture.get();
	const auto queue = cuda::make_test_queue(0);

	auto *block = fixture.add_heap(1024);
	pool.acquire(*block, queue);
	pool.release(*block);

	const auto halves = pool.partition_block(*block, 256, 768);

	REQUIRE( halves.first != nullptr );
	REQUIRE( halves.second != nullptr );
	CHECK( halves.first != halves.second );

	CHECK( halves.first->get_offset() == 0 );
	CHECK( halves.first->get_size() == 256 );
	CHECK( halves.first->is_free() );
	CHECK( halves.first->get_queue() == queue );

	CHECK( halves.second->get_offset() == 256 );
	CHECK( halves.second->get_size() == 768 );
	CHECK( halves.second->is_free() );
	CHECK( halves.second->get_queue() == queue );

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
	const auto queue = cuda::make_test_queue(0);

	auto blocks = partition_evenly(pool, *fixture.add_heap(1024), 4);
	for (auto *block : blocks)
	{
		pool.acquire(*block, queue);
	}

	SECTION( "not at all when both neighbours are still in use" )
	{
		pool.release(*blocks[1]);

		CHECK( blocks[1]->get_size() == 256 );
		CHECK( blocks[1]->get_offset() == 256 );

		pool.release(*blocks[0]);
		pool.release(*blocks[2]);
		pool.release(*blocks[3]);
	}

	SECTION( "forwards" )
	{
		pool.release(*blocks[2]);
		pool.release(*blocks[1]);

		CHECK( blocks[1]->get_size() == 512 );
		CHECK( blocks[1]->get_offset() == 256 );

		pool.release(*blocks[0]);
		pool.release(*blocks[3]);
	}

	SECTION( "backwards" )
	{
		pool.release(*blocks[0]);
		pool.release(*blocks[1]);

		CHECK( blocks[1]->get_size() == 512 );
		CHECK( blocks[1]->get_offset() == 0 );

		pool.release(*blocks[2]);
		pool.release(*blocks[3]);
	}

	SECTION( "both ways at once" )
	{
		pool.release(*blocks[0]);
		pool.release(*blocks[2]);
		pool.release(*blocks[3]);
		pool.release(*blocks[1]);

		CHECK( blocks[1]->get_size() == 1024 );
		CHECK( blocks[1]->get_offset() == 0 );
	}
}

TEST_CASE(
	"a memory_block_pool should not merge blocks a queue would have to wait "
	"for",
	"[memory_block_pool]"
)
{
	pool_fixture fixture;
	auto &pool = fixture.get();
	const auto first_queue = cuda::make_test_queue(0);
	const auto second_queue = cuda::make_test_queue(1);

	const auto blocks = partition_evenly(pool, *fixture.add_heap(1024), 2);
	pool.acquire(*blocks[0], first_queue);
	pool.acquire(*blocks[1], second_queue);

	// Neighbours in one heap, but what has to have finished before each can be
	// handed out is different, so one block out of the two would be a lie.
	pool.release(*blocks[1]);
	pool.release(*blocks[0]);

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

	const auto queue = cuda::make_test_queue(0);
	pool.acquire(*first, queue);
	pool.acquire(*second, queue);
	pool.release(*second);
	pool.release(*first);

	// Two heaps are two calls to the driver and have to be given back as
	// such, whatever the addresses happen to look like.
	CHECK( first->get_size() == 512 );
	CHECK( second->get_size() == 512 );
}

TEST_CASE(
	"resetting the queues of a memory_block_pool should unbind and merge what "
	"the queues kept apart",
	"[memory_block_pool]"
)
{
	pool_fixture fixture;
	auto &pool = fixture.get();
	const auto first_queue = cuda::make_test_queue(0);
	const auto second_queue = cuda::make_test_queue(1);

	const auto blocks = partition_evenly(pool, *fixture.add_heap(1024), 4);
	pool.acquire(*blocks[0], first_queue);
	pool.acquire(*blocks[1], second_queue);
	pool.acquire(*blocks[2], first_queue);
	pool.acquire(*blocks[3], second_queue);
	for (auto *block : blocks)
	{
		pool.release(*block);
	}

	pool.reset_queues();

	// Once nothing is running any more, the four blocks are just the heap
	// again, and any queue can have it.
	auto *found = pool.find_suitable_block(1024, first_queue);
	REQUIRE( found != nullptr );
	CHECK( found->get_size() == 1024 );
	CHECK( found->get_offset() == 0 );
	CHECK_FALSE( static_cast<bool>(found->get_queue()) );
}

TEST_CASE(
	"resetting the queues of a memory_block_pool should leave the blocks in "
	"use alone",
	"[memory_block_pool]"
)
{
	pool_fixture fixture;
	auto &pool = fixture.get();
	const auto queue = cuda::make_test_queue(0);

	const auto blocks = partition_evenly(pool, *fixture.add_heap(1024), 4);
	for (auto *block : blocks)
	{
		pool.acquire(*block, queue);
	}
	pool.release(*blocks[0]);
	pool.release(*blocks[3]);

	pool.reset_queues();

	CHECK( pool.get_acquired_block_count() == 2 );
	CHECK( blocks[1]->get_queue() == queue );
	CHECK( blocks[2]->get_queue() == queue );
	CHECK( blocks[0]->get_size() == 256 );
	CHECK( blocks[3]->get_size() == 256 );

	pool.release(*blocks[1]);
	pool.release(*blocks[2]);
}

TEST_CASE(
	"a memory_block_pool should give back the heaps it is no longer using",
	"[memory_block_pool]"
)
{
	pool_fixture fixture;
	auto &pool = fixture.get();
	const auto queue = cuda::make_test_queue(0);

	SECTION( "when a heap is one whole free block" )
	{
		fixture.add_heap(1024);

		CHECK( pool.release_unused_heaps() == 1024 );
		CHECK( pool.get_size() == 0 );
		CHECK( pool.find_suitable_block(1, queue) == nullptr );
	}

	SECTION( "but not while any of it is in use" )
	{
		auto *block = fixture.add_heap(1024);
		pool.acquire(*block, queue);

		CHECK( pool.release_unused_heaps() == 0 );
		CHECK( pool.get_size() == 1024 );

		pool.release(*block);
	}

	SECTION( "but not while it is still cut up" )
	{
		const auto blocks = partition_evenly(pool, *fixture.add_heap(1024), 2);
		pool.acquire(*blocks[0], queue);

		CHECK( pool.release_unused_heaps() == 0 );
		CHECK( pool.get_size() == 1024 );

		pool.release(*blocks[0]);
	}

	SECTION( "and only the ones that are done with" )
	{
		auto *kept = fixture.add_heap(1024);
		fixture.add_heap(512);
		pool.acquire(*kept, queue);

		CHECK( pool.release_unused_heaps() == 512 );
		CHECK( pool.get_size() == 1024 );

		pool.release(*kept);
	}
}

TEST_CASE(
	"a memory_block_pool should report the queues its free blocks belong to",
	"[memory_block_pool]"
)
{
	pool_fixture fixture;
	auto &pool = fixture.get();
	const auto first_queue = cuda::make_test_queue(0);
	const auto second_queue = cuda::make_test_queue(1);

	const auto blocks = partition_evenly(pool, *fixture.add_heap(1024), 4);
	pool.acquire(*blocks[0], first_queue);
	pool.release(*blocks[0]);
	pool.acquire(*blocks[2], second_queue);
	pool.release(*blocks[2]);

	std::vector<cuda::queue_handle> queues;
	pool.enumerate_queues(queues);

	// The unbound blocks are not listed: there is nothing to wait for on a
	// queue that does not exist.
	REQUIRE( queues.size() == 2 );
	CHECK(
		std::count(queues.cbegin(), queues.cend(), first_queue) == 1
	);
	CHECK(
		std::count(queues.cbegin(), queues.cend(), second_queue) == 1
	);
}
