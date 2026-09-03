// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/caching_image_reader_provider.hpp>

#include "mock/mock_image_reader.hpp"
#include "mock/mock_image_reader_provider.hpp"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

std::shared_ptr<const image_reader> make_reader()
{
	return std::make_shared<mock_image_reader>();
}

} // namespace

TEST_CASE( "a caching reader provider needs a backing provider and room",
	"[caching_image_reader_provider]" )
{
	SECTION( "a null backing provider is refused" )
	{
		REQUIRE_THROWS_AS(
			caching_image_reader_provider(nullptr, 4),
			std::invalid_argument
		);
	}

	SECTION( "a capacity of zero is refused" )
	{
		REQUIRE_THROWS_AS(
			caching_image_reader_provider(
				std::make_shared<mock_image_reader_provider>(),
				0
			),
			std::invalid_argument
		);
	}
}

TEST_CASE( "a caching reader provider asks its backing only on a miss",
	"[caching_image_reader_provider]" )
{
	const auto backing = std::make_shared<mock_image_reader_provider>();
	caching_image_reader_provider provider(backing, 4);

	SECTION( "a miss reaches the backing provider" )
	{
		const auto reader = make_reader();
		REQUIRE_CALL(*backing, acquire("stack_0.mrcs")).RETURN(reader);

		REQUIRE( provider.acquire("stack_0.mrcs") == reader );
		REQUIRE( provider.get_size() == 1 );
	}

	SECTION( "a hit does not" )
	{
		const auto reader = make_reader();
		REQUIRE_CALL(*backing, acquire("stack_0.mrcs")).RETURN(reader)
			.TIMES(1);

		REQUIRE( provider.acquire("stack_0.mrcs") == reader );
		REQUIRE( provider.acquire("stack_0.mrcs") == reader );
		REQUIRE( provider.acquire("stack_0.mrcs") == reader );
	}

	SECTION( "distinct paths are kept apart" )
	{
		const auto first = make_reader();
		const auto second = make_reader();
		REQUIRE_CALL(*backing, acquire("stack_0.mrcs")).RETURN(first);
		REQUIRE_CALL(*backing, acquire("stack_1.mrcs")).RETURN(second);

		REQUIRE( provider.acquire("stack_0.mrcs") == first );
		REQUIRE( provider.acquire("stack_1.mrcs") == second );
		REQUIRE( provider.acquire("stack_0.mrcs") == first );
		REQUIRE( provider.get_size() == 2 );
	}
}

TEST_CASE( "a caching reader provider reopens only what it evicted",
	"[caching_image_reader_provider]" )
{
	const auto backing = std::make_shared<mock_image_reader_provider>();
	caching_image_reader_provider provider(backing, 2);

	REQUIRE( provider.get_capacity() == 2 );

	SECTION( "the least recently asked for is the one evicted" )
	{
		const auto zero = make_reader();
		const auto one = make_reader();
		const auto two = make_reader();

		// stack_0 is asked for again before stack_2 arrives, so stack_1 is
		// the coldest and the one that goes.
		REQUIRE_CALL(*backing, acquire("stack_0.mrcs")).RETURN(zero)
			.TIMES(1);
		REQUIRE_CALL(*backing, acquire("stack_1.mrcs")).RETURN(one)
			.TIMES(2);
		REQUIRE_CALL(*backing, acquire("stack_2.mrcs")).RETURN(two)
			.TIMES(1);

		provider.acquire("stack_0.mrcs");
		provider.acquire("stack_1.mrcs");
		provider.acquire("stack_0.mrcs");
		provider.acquire("stack_2.mrcs");

		REQUIRE( provider.get_size() == 2 );

		provider.acquire("stack_0.mrcs");   // still held
		provider.acquire("stack_1.mrcs");   // evicted, asked again
	}

	SECTION( "it never holds more than its capacity" )
	{
		const auto reader = make_reader();
		ALLOW_CALL(*backing, acquire(trompeloeil::_)).RETURN(reader);

		for (std::size_t i = 0; i < 8; ++i)
		{
			provider.acquire("stack_" + std::to_string(i) + ".mrcs");
			REQUIRE( provider.get_size() <= provider.get_capacity() );
		}

		REQUIRE( provider.get_size() == 2 );
	}
}

TEST_CASE( "an evicted reader stays alive while it is still held",
	"[caching_image_reader_provider]" )
{
	// This is why acquire hands out shared ownership: a transaction reading
	// through a reader must not lose it to an eviction on another thread.
	const auto backing = std::make_shared<mock_image_reader_provider>();
	caching_image_reader_provider provider(backing, 1);

	auto evicted = make_reader();
	const auto *raw = evicted.get();
	std::weak_ptr<const image_reader> observer = evicted;

	const auto other = make_reader();
	ALLOW_CALL(*backing, acquire("stack_0.mrcs")).RETURN(evicted);
	ALLOW_CALL(*backing, acquire("stack_1.mrcs")).RETURN(other);

	const auto held = provider.acquire("stack_0.mrcs");
	evicted.reset();

	provider.acquire("stack_1.mrcs");   // evicts stack_0

	REQUIRE( provider.get_size() == 1 );
	REQUIRE_FALSE( observer.expired() );
	REQUIRE( held.get() == raw );
}
