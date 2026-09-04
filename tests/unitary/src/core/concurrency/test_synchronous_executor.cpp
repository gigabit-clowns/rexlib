// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/concurrency/counting_completion.hpp>
#include <rexlib/core/concurrency/synchronous_executor.hpp>

#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>

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
	"synchronous_executor rejects a null task or notifier",
	"[synchronous_executor]"
)
{
	synchronous_executor executor;
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
	"synchronous_executor runs a task before submit returns",
	"[synchronous_executor]"
)
{
	synchronous_executor executor;
	bool ran = false;

	auto completion = std::make_shared<counting_completion>(1);
	executor.submit(
		std::make_unique<lambda_task>([&ran] { ran = true; }),
		completion
	);

	CHECK( ran );
	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"synchronous_executor catches what a task throws and reports it",
	"[synchronous_executor]"
)
{
	synchronous_executor executor;

	auto completion = std::make_shared<counting_completion>(1);
	executor.submit(
		std::make_unique<lambda_task>(
			[] { throw std::runtime_error("from a task"); }
		),
		completion
	);

	REQUIRE( completion->is_ready() );
	REQUIRE_THROWS_AS( completion->get(), std::runtime_error );
}
