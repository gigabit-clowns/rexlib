// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/concurrency/counting_completion.hpp>
#include <rexlib/core/concurrency/thread_pool_executor.hpp>

#include <rexlib/core/platform/constexpr.hpp>

#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

using namespace rexlib;

namespace
{

class lambda_task final : public task
{
public:
	explicit lambda_task(std::function<void()> body)
		: m_body(std::move(body))
	{
	}

	void run() override
	{
		m_body();
	}

private:
	std::function<void()> m_body;
};

} // namespace

TEST_CASE(
	"thread_pool_executor needs at least one worker",
	"[thread_pool_executor]"
)
{
	REQUIRE_THROWS_AS( thread_pool_executor(0), std::invalid_argument );
}

TEST_CASE(
	"thread_pool_executor rejects a null task or notifier",
	"[thread_pool_executor]"
)
{
	thread_pool_executor executor(1);
	auto completion = std::make_shared<counting_completion>(1);

	SECTION( "a null task" )
	{
		REQUIRE_THROWS_AS(
			executor.submit(nullptr, completion),
			std::invalid_argument
		);
	}

	SECTION( "a null notifier" )
	{
		REQUIRE_THROWS_AS(
			executor.submit(
				std::make_unique<lambda_task>([] {}),
				nullptr
			),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"thread_pool_executor runs a submitted task and reports success",
	"[thread_pool_executor]"
)
{
	thread_pool_executor executor(2);
	std::atomic<bool> ran(false);

	auto completion = std::make_shared<counting_completion>(1);
	executor.submit(
		std::make_unique<lambda_task>([&ran] { ran.store(true); }),
		completion
	);

	completion->wait();

	CHECK( ran.load() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"thread_pool_executor catches what a task throws and reports it",
	"[thread_pool_executor]"
)
{
	thread_pool_executor executor(2);

	auto completion = std::make_shared<counting_completion>(1);
	executor.submit(
		std::make_unique<lambda_task>(
			[] { throw std::runtime_error("from a task"); }
		),
		completion
	);

	completion->wait();

	REQUIRE_THROWS_AS( completion->get(), std::runtime_error );
}

TEST_CASE(
	"thread_pool_executor lets submitted tasks run concurrently",
	"[thread_pool_executor]"
)
{
	// Each task spins until every other one has also entered. A pool that
	// actually serialized tasks would deadlock the first one here rather
	// than merely running slowly, which is what makes this observable
	// rather than timing-dependent.
	REXLIB_CONST_CONSTEXPR std::size_t worker_count = 4;
	thread_pool_executor executor(worker_count);

	std::atomic<std::size_t> entered(0);

	auto completion = std::make_shared<counting_completion>(worker_count);
	std::vector<std::unique_ptr<task>> tasks;
	for (std::size_t i = 0; i < worker_count; ++i)
	{
		tasks.emplace_back(
			std::make_unique<lambda_task>(
				[&entered, worker_count]
				{
					entered.fetch_add(1);
					while (entered.load() < worker_count)
					{
						std::this_thread::yield();
					}
				}
			)
		);
	}

	for (auto &t : tasks)
	{
		executor.submit(std::move(t), completion);
	}

	completion->wait();

	CHECK( entered.load() == worker_count );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"thread_pool_executor exposes its worker count",
	"[thread_pool_executor]"
)
{
	CHECK( thread_pool_executor(3).get_worker_count() == 3 );
}
