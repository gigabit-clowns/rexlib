// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/concurrency/counting_completion.hpp>

#include <rexlib/core/platform/constexpr.hpp>

#include <cstddef>
#include <exception>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

using namespace rexlib;

namespace
{

std::exception_ptr make_error(const std::string &what)
{
	try
	{
		throw std::runtime_error(what);
	}
	catch (...)
	{
		return std::current_exception();
	}
}

} // namespace

TEST_CASE(
	"counting_completion with a count of zero is already resolved",
	"[counting_completion]"
)
{
	counting_completion completion(0);

	CHECK( completion.is_ready() );
	CHECK_NOTHROW( completion.wait() );
	CHECK_NOTHROW( completion.get() );
}

TEST_CASE(
	"counting_completion resolves only once every unit has reported",
	"[counting_completion]"
)
{
	counting_completion completion(3);

	CHECK_FALSE( completion.is_ready() );
	completion.notify_done();
	CHECK_FALSE( completion.is_ready() );
	completion.notify_done();
	CHECK_FALSE( completion.is_ready() );
	completion.notify_done();
	CHECK( completion.is_ready() );

	CHECK_NOTHROW( completion.get() );
}

TEST_CASE(
	"counting_completion get rethrows the first error reported",
	"[counting_completion]"
)
{
	counting_completion completion(3);

	completion.notify_done();
	completion.notify_done(make_error("first"));
	completion.notify_done(make_error("second"));

	REQUIRE( completion.is_ready() );

	try
	{
		completion.get();
		FAIL( "Expected get() to rethrow." );
	}
	catch (const std::runtime_error &e)
	{
		CHECK( std::string(e.what()) == "first" );
	}
}

TEST_CASE(
	"counting_completion get does not throw when nothing failed",
	"[counting_completion]"
)
{
	counting_completion completion(2);

	completion.notify_done();
	completion.notify_done();

	CHECK_NOTHROW( completion.get() );
}

TEST_CASE(
	"counting_completion tolerates concurrent notify_done calls",
	"[counting_completion]"
)
{
	// No assertion on which error wins when several arrive genuinely
	// concurrently: only that resolution happens exactly once, and only
	// after every one of them has reported.
	REXLIB_CONST_CONSTEXPR std::size_t count = 64;
	counting_completion completion(count);

	std::vector<std::thread> reporters;
	reporters.reserve(count);
	for (std::size_t i = 0; i < count; ++i)
	{
		reporters.emplace_back(
			[&completion, i]
			{
				completion.notify_done(
					(i % 8 == 0) ? make_error("concurrent") : nullptr
				);
			}
		);
	}

	for (auto &reporter : reporters)
	{
		reporter.join();
	}

	REQUIRE( completion.is_ready() );
	CHECK_THROWS_AS( completion.get(), std::runtime_error );
}
