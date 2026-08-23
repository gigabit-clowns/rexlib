// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <hardware/caching_allocator/deferred_release.hpp>

#include <hardware/caching_allocator/memory_block.hpp>
#include <hardware/caching_allocator/memory_block_pool.hpp>
#include <hardware/caching_allocator/memory_heap.hpp>

#include "mock/mock_event_recorder.hpp"
#include "mock/mock_memory_source.hpp"
#include "test_queue.hpp"

#include <array>
#include <cstddef>
#include <memory>
#include <stdexcept>

using namespace xmipp4;

namespace
{

/// Stands in for whatever the driver would have handed out.
alignas(256) std::byte storage[1 << 16];

/// A pool over heaps that exist only as addresses.
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
	 * @brief Take a whole heap's worth of block out of the pool.
	 *
	 * @param size Size of the heap, in bytes.
	 * @param queue The queue the block is handed to.
	 * @return cuda::memory_block& The block.
	 */
	cuda::memory_block& acquire_block(
		std::size_t size,
		const cuda::queue_handle &queue
	)
	{
		auto *block = m_pool.register_heap(
			std::make_unique<cuda::memory_heap>(m_source, size, 256)
		);
		m_pool.acquire(*block, queue);
		return *block;
	}

private:
	cuda::mock_memory_source m_source;
	std::size_t m_used;

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

/**
 * @brief A deferred release over a pool of blocks that exist only as
 * addresses.
 *
 * Stands in for the owner a deferred release expects, draining it while there
 * is still a pool to give the blocks back to. Waiting and giving tickets back
 * are allowed throughout, since that drain causes both and every test would
 * otherwise have to say so. A test that is about those calls says so itself,
 * which takes precedence.
 */
class deferred_fixture
{
public:
	deferred_fixture()
		: m_wait_expectation(
			NAMED_ALLOW_CALL(m_recorder, wait(trompeloeil::_))
		)
		, m_release_expectation(
			NAMED_ALLOW_CALL(m_recorder, release(trompeloeil::_))
		)
	{
	}

	cuda::deferred_release& get() noexcept
	{
		return m_deferred;
	}

	cuda::mock_event_recorder& get_recorder() noexcept
	{
		return m_recorder;
	}

	~deferred_fixture()
	{
		m_deferred.wait_all(m_pool.get());
	}

	cuda::memory_block_pool& get_pool() noexcept
	{
		return m_pool.get();
	}

	cuda::memory_block& acquire_block(
		std::size_t size,
		const cuda::queue_handle &queue
	)
	{
		return m_pool.acquire_block(size, queue);
	}

private:
	pool_fixture m_pool;
	cuda::mock_event_recorder m_recorder;

	// Declared before the deferred release, so that they are destroyed after
	// it and are still around while it drains.
	std::unique_ptr<trompeloeil::expectation> m_wait_expectation;
	std::unique_ptr<trompeloeil::expectation> m_release_expectation;

	cuda::deferred_release m_deferred;
};

} // namespace

TEST_CASE(
	"deferred_release should hold a block back until every queue that used it "
	"has caught up",
	"[deferred_release]"
)
{
	deferred_fixture fixture;
	auto &recorder = fixture.get_recorder();
	auto &deferred = fixture.get();

	const auto own = cuda::make_test_queue(0);
	const auto first = cuda::make_test_queue(1);
	const auto second = cuda::make_test_queue(2);
	auto &block = fixture.acquire_block(1024, own);

	// One point per queue that touched the block besides its own.
	REQUIRE_CALL(recorder, record(first))
		.RETURN(1);
	REQUIRE_CALL(recorder, record(second))
		.RETURN(2);

	const std::array<cuda::queue_handle, 2> queues = {first, second};
	deferred.defer(recorder, block, make_span(queues));

	CHECK( deferred.get_pending_count() == 1 );
	CHECK_FALSE( block.is_free() );

	SECTION( "staying held while neither has" )
	{
		ALLOW_CALL(recorder, is_complete(trompeloeil::_))
			.RETURN(false);

		deferred.process(fixture.get_pool());

		CHECK( deferred.get_pending_count() == 1 );
		CHECK_FALSE( block.is_free() );
	}

	SECTION( "staying held while only one has" )
	{
		ALLOW_CALL(recorder, is_complete(1u))
			.RETURN(true);
		ALLOW_CALL(recorder, is_complete(2u))
			.RETURN(false);

		// The point that has been reached is given back straight away rather
		// than being queried again on every sweep.
		REQUIRE_CALL(recorder, release(1u));

		deferred.process(fixture.get_pool());

		CHECK( deferred.get_pending_count() == 1 );
		CHECK_FALSE( block.is_free() );
	}

	SECTION( "and going back once both have" )
	{
		ALLOW_CALL(recorder, is_complete(trompeloeil::_))
			.RETURN(true);
		REQUIRE_CALL(recorder, release(1u));
		REQUIRE_CALL(recorder, release(2u));

		deferred.process(fixture.get_pool());

		CHECK( deferred.get_pending_count() == 0 );
		CHECK( block.is_free() );
	}
}

TEST_CASE(
	"a block given back by deferred_release should merge with its neighbours",
	"[deferred_release]"
)
{
	deferred_fixture fixture;
	auto &recorder = fixture.get_recorder();
	auto &deferred = fixture.get();
	auto &pool = fixture.get_pool();

	const auto own = cuda::make_test_queue(0);
	const auto other = cuda::make_test_queue(1);

	auto &whole = fixture.acquire_block(1024, own);
	pool.release(whole);
	const auto halves = pool.partition_block(whole, 512, 512);
	pool.acquire(*halves.first, own);
	pool.acquire(*halves.second, own);
	pool.release(*halves.second);

	ALLOW_CALL(recorder, record(other))
		.RETURN(1);
	ALLOW_CALL(recorder, is_complete(1u))
		.RETURN(true);

	const std::array<cuda::queue_handle, 1> queues = {other};
	deferred.defer(recorder, *halves.first, make_span(queues));

	// While it waits it is not free, so the neighbour that came back before it
	// could not have swallowed it.
	CHECK( halves.second->get_size() == 512 );

	deferred.process(fixture.get_pool());

	CHECK( halves.first->get_size() == 1024 );
	CHECK( halves.first->get_offset() == 0 );
}

TEST_CASE(
	"waiting on a deferred_release should give every block back",
	"[deferred_release]"
)
{
	deferred_fixture fixture;
	auto &recorder = fixture.get_recorder();
	auto &deferred = fixture.get();

	const auto own = cuda::make_test_queue(0);
	const auto other = cuda::make_test_queue(1);
	auto &first = fixture.acquire_block(1024, own);
	auto &second = fixture.acquire_block(1024, own);

	ALLOW_CALL(recorder, record(other))
		.RETURN(1);

	const std::array<cuda::queue_handle, 1> queues = {other};
	deferred.defer(recorder, first, make_span(queues));
	deferred.defer(recorder, second, make_span(queues));

	REQUIRE_CALL(recorder, wait(1u))
		.TIMES(2);
	REQUIRE_CALL(recorder, release(1u))
		.TIMES(2);

	deferred.wait_all(fixture.get_pool());

	CHECK( deferred.get_pending_count() == 0 );
	CHECK( first.is_free() );
	CHECK( second.is_free() );
}

TEST_CASE(
	"a deferred_release should wait for what it is still holding when it dies",
	"[deferred_release]"
)
{
	pool_fixture pool;
	cuda::mock_event_recorder recorder;

	const auto own = cuda::make_test_queue(0);
	const auto other = cuda::make_test_queue(1);
	auto &block = pool.acquire_block(1024, own);

	ALLOW_CALL(recorder, record(other))
		.RETURN(1);

	// Letting the pool go while a queue may still be reading from one of its
	// heaps would hand that memory back to the driver underneath it.
	REQUIRE_CALL(recorder, wait(1u));
	REQUIRE_CALL(recorder, release(1u));

	{
		cuda::deferred_release deferred;

		const std::array<cuda::queue_handle, 1> queues = {other};
		deferred.defer(recorder, block, make_span(queues));

		// Draining is the owner's to do, while it still has the pool to give
		// the blocks back to.
		deferred.wait_all(pool.get());
	}

	CHECK( block.is_free() );
}

TEST_CASE(
	"deferred_release should refuse to hold a block back for nothing",
	"[deferred_release]"
)
{
	deferred_fixture fixture;
	auto &recorder = fixture.get_recorder();
	auto &deferred = fixture.get();

	const auto own = cuda::make_test_queue(0);
	auto &block = fixture.acquire_block(1024, own);

	FORBID_CALL(recorder, record(trompeloeil::_));

	SECTION( "when no queue is named" )
	{
		CHECK_THROWS_AS(
			deferred.defer(recorder, block, span<const cuda::queue_handle>()),
			std::invalid_argument
		);
	}

	SECTION( "when a queue that does not exist is named" )
	{
		const std::array<cuda::queue_handle, 1> queues =
			{cuda::queue_handle()};
		CHECK_THROWS_AS(
			deferred.defer(recorder, block, make_span(queues)),
			std::invalid_argument
		);
	}

	CHECK( deferred.get_pending_count() == 0 );

	fixture.get_pool().release(block);
}

TEST_CASE(
	"a deferred_release should give back what it captured when capturing "
	"fails part way through",
	"[deferred_release]"
)
{
	deferred_fixture fixture;
	auto &recorder = fixture.get_recorder();
	auto &deferred = fixture.get();

	const auto own = cuda::make_test_queue(0);
	const auto first = cuda::make_test_queue(1);
	const auto second = cuda::make_test_queue(2);
	auto &block = fixture.acquire_block(1024, own);

	REQUIRE_CALL(recorder, record(first))
		.RETURN(1);
	REQUIRE_CALL(recorder, record(second))
		.THROW(std::runtime_error("no more events"));

	// Half a wait is worse than none: the block would look like it only ever
	// needed the one queue to catch up.
	REQUIRE_CALL(recorder, release(1u));

	const std::array<cuda::queue_handle, 2> queues = {first, second};
	CHECK_THROWS_AS(
		deferred.defer(recorder, block, make_span(queues)),
		std::runtime_error
	);

	CHECK( deferred.get_pending_count() == 0 );
	CHECK_FALSE( block.is_free() );

	fixture.get_pool().release(block);
}

TEST_CASE(
	"a deferred_release should give a block back once when waiting fails part "
	"way through",
	"[deferred_release]"
)
{
	pool_fixture pool;
	cuda::mock_event_recorder recorder;

	const auto own = cuda::make_test_queue(0);
	const auto other = cuda::make_test_queue(1);

	// One heap each, so that the one that goes back cannot merge into the one
	// that does not and stop being a block of its own.
	auto &first = pool.acquire_block(1024, own);
	auto &second = pool.acquire_block(1024, own);

	trompeloeil::sequence sequence;
	REQUIRE_CALL(recorder, record(other))
		.RETURN(1)
		.IN_SEQUENCE(sequence);
	REQUIRE_CALL(recorder, record(other))
		.RETURN(2)
		.IN_SEQUENCE(sequence);
	ALLOW_CALL(recorder, release(trompeloeil::_));

	cuda::deferred_release deferred;
	const std::array<cuda::queue_handle, 1> queues = {other};
	deferred.defer(recorder, first, make_span(queues));
	deferred.defer(recorder, second, make_span(queues));

	// The first block's queue catches up; the second one's cannot be waited
	// for at all.
	ALLOW_CALL(recorder, wait(1u));
	ALLOW_CALL(recorder, wait(2u))
		.THROW(std::runtime_error("no answer"));

	CHECK_THROWS_AS(
		deferred.wait_all(pool.get()),
		std::runtime_error
	);

	// The one that made it back is no longer held back, so the drain the owner
	// still has to do cannot hand it to the pool a second time.
	CHECK( first.is_free() );
	CHECK_FALSE( second.is_free() );
	CHECK( deferred.get_pending_count() == 1 );

	ALLOW_CALL(recorder, wait(2u));
	deferred.wait_all(pool.get());

	CHECK( second.is_free() );
	CHECK( deferred.get_pending_count() == 0 );
}

TEST_CASE(
	"a deferred_release should leave a block waiting for every point it was "
	"waiting for when one cannot be queried",
	"[deferred_release]"
)
{
	deferred_fixture fixture;
	auto &recorder = fixture.get_recorder();
	auto &deferred = fixture.get();

	const auto own = cuda::make_test_queue(0);
	const auto first = cuda::make_test_queue(1);
	const auto second = cuda::make_test_queue(2);
	auto &block = fixture.acquire_block(1024, own);

	ALLOW_CALL(recorder, record(first))
		.RETURN(1);
	ALLOW_CALL(recorder, record(second))
		.RETURN(2);

	const std::array<cuda::queue_handle, 2> queues = {first, second};
	deferred.defer(recorder, block, make_span(queues));

	{
		// The first point has been reached, the second cannot be asked about.
		ALLOW_CALL(recorder, is_complete(1u))
			.RETURN(true);
		ALLOW_CALL(recorder, is_complete(2u))
			.THROW(std::runtime_error("no answer"));

		// Giving the reached one back here would leave the block waiting for a
		// subset of what it was waiting for.
		FORBID_CALL(recorder, release(trompeloeil::_));

		CHECK_THROWS_AS(
			deferred.process(fixture.get_pool()),
			std::runtime_error
		);

		CHECK( deferred.get_pending_count() == 1 );
		CHECK_FALSE( block.is_free() );
	}

	// Both points are still there to be asked about once the driver answers.
	REQUIRE_CALL(recorder, is_complete(1u))
		.RETURN(true);
	REQUIRE_CALL(recorder, is_complete(2u))
		.RETURN(true);
	REQUIRE_CALL(recorder, release(1u));
	REQUIRE_CALL(recorder, release(2u));

	deferred.process(fixture.get_pool());

	CHECK( deferred.get_pending_count() == 0 );
	CHECK( block.is_free() );
}
