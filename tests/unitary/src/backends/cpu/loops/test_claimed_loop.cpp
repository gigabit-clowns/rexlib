// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/loops/claimed_loop.hpp>

#include <rexlib/backends/cpu/thread_pool.hpp>

#include <atomic>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace rexlib::cpu;

TEST_CASE(
	"run_claimed_loop should run every iteration exactly once",
	"[claimed_loop]"
)
{
	thread_pool pool(3);
	const std::size_t count = 1000;
	std::vector<std::unique_ptr<std::atomic<int>>> visits;
	for (std::size_t i = 0; i < count; ++i)
	{
		visits.push_back(std::unique_ptr<std::atomic<int>>(
			new std::atomic<int>(0)
		));
	}

	run_claimed_loop(
		pool,
		count,
		[&](std::size_t index)
		{
			visits[index]->fetch_add(1);
		}
	);

	for (const auto &visit : visits)
	{
		CHECK( visit->load() == 1 );
	}
}

TEST_CASE(
	"run_claimed_loop should run nothing for a count of 0",
	"[claimed_loop]"
)
{
	thread_pool pool(0);
	std::size_t calls = 0;

	run_claimed_loop(pool, 0, [&](std::size_t) { ++calls; });

	CHECK( calls == 0 );
}

TEST_CASE(
	"run_claimed_loop should rethrow what an iteration throws",
	"[claimed_loop]"
)
{
	thread_pool pool(2);

	CHECK_THROWS_AS(
		run_claimed_loop(
			pool,
			8,
			[](std::size_t index)
			{
				if (index == 5)
				{
					throw std::runtime_error("five");
				}
			}
		),
		std::runtime_error
	);
}
