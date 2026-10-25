// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/backends/cpu/parallel_for.hpp>
#include <rexlib/backends/cpu/thread_pool.hpp>

#include <rexlib/core/platform/constexpr.hpp>

#include <trompeloeil.hpp>

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{

class mock_body
{
public:
	MAKE_CONST_MOCK2(run, void(std::size_t begin, std::size_t end));
};

// A body for parallel_for that hands each chunk to the mock.
auto forward_to(const mock_body &body)
{
	return [&body] (std::size_t begin, std::size_t end)
	{
		body.run(begin, end);
	};
}

} // anonymous namespace


TEST_CASE(
	"thread_pool should count the calling thread among its participants",
	"[thread_pool]"
)
{
	// A pool of no worker still has one participant, so that a caller never
	// has to special case it.
	CHECK( thread_pool(0).get_size() == 1 );
	CHECK( thread_pool(3).get_size() == 4 );
}

TEST_CASE(
	"parallel_for should run the whole range on the calling thread when the "
	"pool has no worker",
	"[thread_pool]"
)
{
	thread_pool pool(0);

	const mock_body body;
	const auto caller = std::this_thread::get_id();
	REQUIRE_CALL(body, run(0u, 1000u))
		.LR_WITH(std::this_thread::get_id() == caller);

	parallel_for(pool, 1000, 1, forward_to(body));
}

TEST_CASE(
	"parallel_for should cut the range into one chunk per participant",
	"[thread_pool]"
)
{
	thread_pool pool(3);

	const mock_body body;
	REQUIRE_CALL(body, run(0u, 25u));
	REQUIRE_CALL(body, run(25u, 50u));
	REQUIRE_CALL(body, run(50u, 75u));
	REQUIRE_CALL(body, run(75u, 100u));

	parallel_for(pool, 100, 1, forward_to(body));
}

TEST_CASE(
	"parallel_for should balance a range that does not divide evenly",
	"[thread_pool]"
)
{
	// 10 over 4 participants is two chunks of 3 and two of 2, the longer ones
	// first. The partition depends only on the count, the grain and the pool
	// size, never on timing, so it can be asserted exactly.
	thread_pool pool(3);

	const mock_body body;
	REQUIRE_CALL(body, run(0u, 3u));
	REQUIRE_CALL(body, run(3u, 6u));
	REQUIRE_CALL(body, run(6u, 8u));
	REQUIRE_CALL(body, run(8u, 10u));

	parallel_for(pool, 10, 1, forward_to(body));
}

TEST_CASE(
	"parallel_for should not split work below one grain",
	"[thread_pool]"
)
{
	thread_pool pool(3);

	const mock_body body;
	REQUIRE_CALL(body, run(0u, 100u));

	parallel_for(pool, 100, 1000, forward_to(body));
}

TEST_CASE(
	"parallel_for should give every chunk at least one grain",
	"[thread_pool]"
)
{
	// 100 iterations at a grain of 40 affords two chunks, not the four the
	// pool could otherwise fill.
	thread_pool pool(3);

	const mock_body body;
	REQUIRE_CALL(body, run(0u, 50u));
	REQUIRE_CALL(body, run(50u, 100u));

	parallel_for(pool, 100, 40, forward_to(body));
}

TEST_CASE(
	"parallel_for should not invoke the body for an empty range",
	"[thread_pool]"
)
{
	thread_pool pool(3);

	const mock_body body;
	FORBID_CALL(body, run(trompeloeil::_, trompeloeil::_));

	parallel_for(pool, 0, 1, forward_to(body));
}

TEST_CASE(
	"parallel_for should visit every iteration exactly once",
	"[thread_pool]"
)
{
	REXLIB_CONST_CONSTEXPR std::size_t count = 10000;
	thread_pool pool(4);

	std::vector<int> visits(count, 0);
	const auto body =
		[&visits] (std::size_t begin, std::size_t end)
		{
			for (std::size_t i = begin; i < end; ++i)
			{
				++visits[i];
			}
		};

	parallel_for(pool, count, 1, body);

	CHECK( std::count(visits.cbegin(), visits.cend(), 1) ==
	       static_cast<std::ptrdiff_t>(count) );
}

TEST_CASE(
	"parallel_for should be reusable for one job after another",
	"[thread_pool]"
)
{
	// The workers sleep between jobs, so this is what catches a wake-up lost
	// to a worker that was still draining the previous one.
	thread_pool pool(4);

	std::atomic<std::size_t> total(0);
	const auto body =
		[&total] (std::size_t begin, std::size_t end)
		{
			total.fetch_add(end - begin);
		};

	for (std::size_t i = 0; i < 1000; ++i)
	{
		parallel_for(pool, 100, 1, body);
	}

	CHECK( total.load() == 100*1000 );
}

TEST_CASE(
	"parallel_for should run a nested loop inline rather than deadlock",
	"[thread_pool]"
)
{
	// A job holds the pool for the whole of its duration, so the inner loop
	// finds it taken and is handed its whole range on the thread that asked
	// for it. Waiting instead would be waiting on the very threads that are
	// running this body. Repeated, because the failure it guards against is
	// a race rather than a misreading.
	thread_pool pool(3);

	std::atomic<std::size_t> inner_calls(0);
	std::atomic<std::size_t> inner_total(0);
	const auto body =
		[&pool, &inner_calls, &inner_total] (std::size_t, std::size_t)
		{
			const auto inner =
				[&inner_calls, &inner_total]
				(std::size_t begin, std::size_t end)
				{
					inner_calls.fetch_add(1);
					inner_total.fetch_add(end - begin);
				};

			parallel_for(pool, 100, 1, inner);
		};

	for (std::size_t i = 0; i < 100; ++i)
	{
		parallel_for(pool, 4, 1, body);
	}

	CHECK( inner_calls.load() == 4*100 );
	CHECK( inner_total.load() == 100*4*100 );
}

TEST_CASE(
	"parallel_for should rethrow what the body threw on the calling thread",
	"[thread_pool]"
)
{
	thread_pool pool(3);

	const auto body =
		[] (std::size_t, std::size_t)
		{
			throw std::runtime_error("from a chunk");
		};

	REQUIRE_THROWS_AS( parallel_for(pool, 100, 1, body), std::runtime_error );

	SECTION("and the pool is usable afterwards")
	{
		std::atomic<std::size_t> total(0);
		const auto counter =
			[&total] (std::size_t begin, std::size_t end)
			{
				total.fetch_add(end - begin);
			};

		REQUIRE_NOTHROW( parallel_for(pool, 100, 1, counter) );
		CHECK( total.load() == 100 );
	}
}

TEST_CASE(
	"parallel_for should report the lowest numbered chunk that threw",
	"[thread_pool]"
)
{
	// Every chunk throws, so which exception survives is decided by the rule
	// rather than by which core got there first. The lowest one wins, which
	// is what makes the message a user sees reproducible.
	thread_pool pool(3);

	const auto body =
		[] (std::size_t begin, std::size_t)
		{
			throw std::logic_error(std::to_string(begin));
		};

	for (std::size_t i = 0; i < 100; ++i)
	{
		try
		{
			parallel_for(pool, 100, 1, body);
			FAIL( "Expected the body's exception to propagate." );
		}
		catch (const std::logic_error &e)
		{
			CHECK( std::string(e.what()) == "0" );
		}
	}
}

TEST_CASE(
	"thread_pool::get_default_worker_count should leave room for the caller",
	"[thread_pool]"
)
{
	// One participant per hardware thread, the caller being one of them, so
	// the count is one short of a full machine and a single core asks for no
	// worker at all. Nothing stronger can be checked without pinning down the
	// machine the case runs on.
	CHECK_NOTHROW( thread_pool::get_default_worker_count() );
	CHECK(
		thread_pool(thread_pool::get_default_worker_count()).get_size() >= 1
	);
}
