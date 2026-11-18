// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <hardware/caching_allocator/deferred_release.hpp>

#include <hardware/caching_allocator/memory_block.hpp>
#include <hardware/caching_allocator/memory_block_pool.hpp>
#include <hardware/caching_allocator/memory_heap.hpp>

#include "../mock/mock_command_timeline.hpp"
#include "mock/mock_memory_source.hpp"

#include <rexlib/core/hardware/command_token.hpp>
#include <rexlib/core/span.hpp>

#include <array>
#include <cstddef>
#include <memory>
#include <stdexcept>

using namespace rexlib;

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
	 * @return cuda::memory_block& The block.
	 */
	cuda::memory_block& acquire_block(std::size_t size)
	{
		auto *block = m_pool.register_heap(
			std::make_unique<cuda::memory_heap>(m_source, size, 256)
		);
		m_pool.acquire(*block);
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
 * addresses, and two timelines whose commands use them.
 *
 * Stands in for the owner a deferred release expects, draining it while there
 * is still a pool to give the blocks back to. Waiting is allowed throughout,
 * since that drain causes it and every test would otherwise have to say so.
 * A test that is about those calls says so itself, which takes precedence.
 */
class deferred_fixture
{
public:
	deferred_fixture()
		: m_first(std::make_shared<cuda::mock_command_timeline>())
		, m_second(std::make_shared<cuda::mock_command_timeline>())
		, m_first_wait_expectation(
			NAMED_ALLOW_CALL(*m_first, wait(trompeloeil::_))
		)
		, m_second_wait_expectation(
			NAMED_ALLOW_CALL(*m_second, wait(trompeloeil::_))
		)
	{
	}

	~deferred_fixture()
	{
		m_deferred.wait_all(m_pool.get());
	}

	cuda::deferred_release& get() noexcept
	{
		return m_deferred;
	}

	cuda::memory_block_pool& get_pool() noexcept
	{
		return m_pool.get();
	}

	const std::shared_ptr<cuda::mock_command_timeline>&
	get_first_timeline() const noexcept
	{
		return m_first;
	}

	const std::shared_ptr<cuda::mock_command_timeline>&
	get_second_timeline() const noexcept
	{
		return m_second;
	}

	cuda::memory_block& acquire_block(std::size_t size)
	{
		return m_pool.acquire_block(size);
	}

private:
	pool_fixture m_pool;
	std::shared_ptr<cuda::mock_command_timeline> m_first;
	std::shared_ptr<cuda::mock_command_timeline> m_second;

	// Declared before the deferred release, so that they are destroyed after
	// it and are still around while it drains.
	std::unique_ptr<trompeloeil::expectation> m_first_wait_expectation;
	std::unique_ptr<trompeloeil::expectation> m_second_wait_expectation;

	cuda::deferred_release m_deferred;
};

} // anonymous namespace

TEST_CASE(
	"deferred_release should hold a block back until at most one of the "
	"commands that use it is still running",
	"[deferred_release]"
)
{
	deferred_fixture fixture;
	auto &deferred = fixture.get();
	const auto &first = fixture.get_first_timeline();
	const auto &second = fixture.get_second_timeline();
	auto &block = fixture.acquire_block(1024);

	const std::array<command_token, 2> tokens = {
		command_token(first, 1),
		command_token(second, 1)
	};
	deferred.defer(block, make_span(tokens));

	CHECK( deferred.get_pending_count() == 1 );
	CHECK_FALSE( block.is_free() );

	SECTION( "staying held while both are" )
	{
		ALLOW_CALL(*first, is_complete(1u))
			.RETURN(false);
		ALLOW_CALL(*second, is_complete(1u))
			.RETURN(false);

		deferred.process(fixture.get_pool());

		CHECK( deferred.get_pending_count() == 1 );
		CHECK_FALSE( block.is_free() );
	}

	SECTION( "going back with the token of the one that is left" )
	{
		ALLOW_CALL(*first, is_complete(1u))
			.RETURN(true);
		ALLOW_CALL(*second, is_complete(1u))
			.RETURN(false);

		deferred.process(fixture.get_pool());

		// A free block carries one token, and one is all that is left to
		// wait for.
		CHECK( deferred.get_pending_count() == 0 );
		CHECK( block.is_free() );
		CHECK( block.get_pending().get_timeline() == second );
		CHECK( block.get_pending().get_id() == 1 );
	}

	SECTION( "and going back with an empty token once both have finished" )
	{
		ALLOW_CALL(*first, is_complete(1u))
			.RETURN(true);
		ALLOW_CALL(*second, is_complete(1u))
			.RETURN(true);

		deferred.process(fixture.get_pool());

		CHECK( deferred.get_pending_count() == 0 );
		CHECK( block.is_free() );
		CHECK( block.get_pending().is_empty() );
	}
}

TEST_CASE(
	"a block given back by deferred_release should merge with its neighbours",
	"[deferred_release]"
)
{
	deferred_fixture fixture;
	auto &deferred = fixture.get();
	auto &pool = fixture.get_pool();
	const auto &first = fixture.get_first_timeline();
	const auto &second = fixture.get_second_timeline();

	auto &whole = fixture.acquire_block(1024);
	pool.release(whole, command_token());
	const auto halves = pool.partition_block(whole, 512, 512);
	pool.acquire(*halves.first);
	pool.acquire(*halves.second);
	pool.release(*halves.second, command_token());

	ALLOW_CALL(*first, is_complete(1u))
		.RETURN(true);
	ALLOW_CALL(*second, is_complete(1u))
		.RETURN(true);

	const std::array<command_token, 2> tokens = {
		command_token(first, 1),
		command_token(second, 1)
	};
	deferred.defer(*halves.first, make_span(tokens));

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
	auto &deferred = fixture.get();
	const auto &first = fixture.get_first_timeline();
	const auto &second = fixture.get_second_timeline();

	auto &first_block = fixture.acquire_block(1024);
	auto &second_block = fixture.acquire_block(1024);

	const std::array<command_token, 2> tokens = {
		command_token(first, 1),
		command_token(second, 1)
	};
	deferred.defer(first_block, make_span(tokens));
	deferred.defer(second_block, make_span(tokens));

	REQUIRE_CALL(*first, wait(1u))
		.TIMES(2);
	REQUIRE_CALL(*second, wait(1u))
		.TIMES(2);

	deferred.wait_all(fixture.get_pool());

	// Everything has been waited for, so there is nothing left to carry.
	CHECK( deferred.get_pending_count() == 0 );
	CHECK( first_block.is_free() );
	CHECK( second_block.is_free() );
	CHECK( first_block.get_pending().is_empty() );
	CHECK( second_block.get_pending().is_empty() );
}

TEST_CASE(
	"a deferred_release should be drained by its owner before it dies",
	"[deferred_release]"
)
{
	pool_fixture pool;
	const auto first = std::make_shared<cuda::mock_command_timeline>();
	const auto second = std::make_shared<cuda::mock_command_timeline>();
	auto &block = pool.acquire_block(1024);

	// Letting the pool go while a queue may still be reading from one of its
	// heaps would hand that memory back to the driver underneath it.
	REQUIRE_CALL(*first, wait(1u));
	REQUIRE_CALL(*second, wait(1u));

	{
		cuda::deferred_release deferred;

		const std::array<command_token, 2> tokens = {
			command_token(first, 1),
			command_token(second, 1)
		};
		deferred.defer(block, make_span(tokens));

		// Draining is the owner's to do, while it still has the pool to give
		// the blocks back to.
		deferred.wait_all(pool.get());
	}

	CHECK( block.is_free() );
}

TEST_CASE(
	"deferred_release should refuse a block that a single token can speak "
	"for",
	"[deferred_release]"
)
{
	deferred_fixture fixture;
	auto &deferred = fixture.get();
	auto &block = fixture.acquire_block(1024);

	SECTION( "when no command uses it" )
	{
		CHECK_THROWS_AS(
			deferred.defer(block, span<const command_token>()),
			std::invalid_argument
		);
	}

	SECTION( "when one command uses it" )
	{
		const std::array<command_token, 1> tokens = {
			command_token(fixture.get_first_timeline(), 1)
		};
		CHECK_THROWS_AS(
			deferred.defer(block, make_span(tokens)),
			std::invalid_argument
		);
	}

	CHECK( deferred.get_pending_count() == 0 );
	CHECK_FALSE( block.is_free() );

	fixture.get_pool().release(block, command_token());
}

TEST_CASE(
	"a deferred_release should give a block back once when waiting fails part "
	"way through",
	"[deferred_release]"
)
{
	pool_fixture pool;
	const auto first = std::make_shared<cuda::mock_command_timeline>();
	const auto second = std::make_shared<cuda::mock_command_timeline>();

	// One heap each, so that the one that goes back cannot merge into the one
	// that does not and stop being a block of its own.
	auto &first_block = pool.acquire_block(1024);
	auto &second_block = pool.acquire_block(1024);

	cuda::deferred_release deferred;
	const std::array<command_token, 2> first_tokens = {
		command_token(first, 1),
		command_token(second, 1)
	};
	const std::array<command_token, 2> second_tokens = {
		command_token(first, 2),
		command_token(second, 2)
	};
	deferred.defer(first_block, make_span(first_tokens));
	deferred.defer(second_block, make_span(second_tokens));

	// The commands of the first block finish; one of the second block's
	// cannot be waited for at all.
	ALLOW_CALL(*first, wait(trompeloeil::_));
	ALLOW_CALL(*second, wait(1u));
	{
		ALLOW_CALL(*second, wait(2u))
			.THROW(std::runtime_error("no answer"));

		CHECK_THROWS_AS(
			deferred.wait_all(pool.get()),
			std::runtime_error
		);
	}

	// The one that made it back is no longer held back, so the drain the owner
	// still has to do cannot hand it to the pool a second time.
	CHECK( first_block.is_free() );
	CHECK_FALSE( second_block.is_free() );
	CHECK( deferred.get_pending_count() == 1 );

	ALLOW_CALL(*second, wait(2u));
	deferred.wait_all(pool.get());

	CHECK( second_block.is_free() );
	CHECK( deferred.get_pending_count() == 0 );
}

TEST_CASE(
	"a deferred_release should leave a block waiting for every token it was "
	"waiting for when one cannot be queried",
	"[deferred_release]"
)
{
	deferred_fixture fixture;
	auto &deferred = fixture.get();
	const auto &first = fixture.get_first_timeline();
	const auto &second = fixture.get_second_timeline();
	auto &block = fixture.acquire_block(1024);

	const std::array<command_token, 2> tokens = {
		command_token(first, 1),
		command_token(second, 1)
	};
	deferred.defer(block, make_span(tokens));

	{
		// The first command has finished, the second cannot be asked about.
		ALLOW_CALL(*first, is_complete(1u))
			.RETURN(true);
		ALLOW_CALL(*second, is_complete(1u))
			.THROW(std::runtime_error("no answer"));

		CHECK_THROWS_AS(
			deferred.process(fixture.get_pool()),
			std::runtime_error
		);

		// Forgetting the finished one here would have the block go back
		// carrying a token nobody was able to ask about.
		CHECK( deferred.get_pending_count() == 1 );
		CHECK_FALSE( block.is_free() );
	}

	// Both tokens are still there to be asked about once the driver answers.
	REQUIRE_CALL(*first, is_complete(1u))
		.RETURN(true);
	REQUIRE_CALL(*second, is_complete(1u))
		.RETURN(true);

	deferred.process(fixture.get_pool());

	CHECK( deferred.get_pending_count() == 0 );
	CHECK( block.is_free() );
}
