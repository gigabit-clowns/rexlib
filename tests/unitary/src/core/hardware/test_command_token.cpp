// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rexlib/core/hardware/command_token.hpp>

#include "mock/mock_command_timeline.hpp"

#include <cstddef>
#include <memory>
#include <utility>

using namespace rexlib;

TEST_CASE(
	"command_token default constructor produces an empty token",
	"[command_token]"
)
{
	const command_token token;
	CHECK( token.is_empty() );
	CHECK( token.get_timeline() == nullptr );
	CHECK( token.is_complete() );
	REQUIRE_NOTHROW( token.wait() );
}

TEST_CASE(
	"command_token constructor stores the timeline and the id",
	"[command_token]"
)
{
	const auto timeline = std::make_shared<mock_command_timeline>();
	const std::size_t id = 42;

	const command_token token(timeline, id);

	CHECK_FALSE( token.is_empty() );
	CHECK( token.get_timeline() == timeline );
	CHECK( token.get_id() == id );
}

TEST_CASE(
	"command_token with a null timeline is empty",
	"[command_token]"
)
{
	const command_token token(nullptr, 42);
	CHECK( token.is_empty() );
	CHECK( token.is_complete() );
	REQUIRE_NOTHROW( token.wait() );
}

TEST_CASE(
	"command_token::is_complete asks its timeline about its id",
	"[command_token]"
)
{
	const auto timeline = std::make_shared<mock_command_timeline>();
	const std::size_t id = 7;
	const command_token token(timeline, id);

	const auto expected = GENERATE(false, true);
	REQUIRE_CALL(*timeline, is_complete(id))
		.RETURN(expected);

	CHECK( token.is_complete() == expected );
}

TEST_CASE(
	"command_token::wait waits on its timeline for its id",
	"[command_token]"
)
{
	const auto timeline = std::make_shared<mock_command_timeline>();
	const std::size_t id = 7;
	const command_token token(timeline, id);

	REQUIRE_CALL(*timeline, wait(id));

	token.wait();
}

TEST_CASE(
	"command_token copies stand for the same command",
	"[command_token]"
)
{
	const auto timeline = std::make_shared<mock_command_timeline>();
	const std::size_t id = 3;
	const command_token original(timeline, id);

	SECTION("copy constructor")
	{
		const command_token copy(original);
		CHECK( copy.get_timeline() == timeline );
		CHECK( copy.get_id() == id );
	}

	SECTION("copy assignment")
	{
		command_token copy;
		copy = original;
		CHECK( copy.get_timeline() == timeline );
		CHECK( copy.get_id() == id );
	}
}

TEST_CASE(
	"command_token move transfers the timeline and the id",
	"[command_token]"
)
{
	const auto timeline = std::make_shared<mock_command_timeline>();
	const std::size_t id = 3;
	command_token original(timeline, id);

	SECTION("move constructor")
	{
		const command_token moved(std::move(original));
		CHECK( moved.get_timeline() == timeline );
		CHECK( moved.get_id() == id );
	}

	SECTION("move assignment")
	{
		command_token moved;
		moved = std::move(original);
		CHECK( moved.get_timeline() == timeline );
		CHECK( moved.get_id() == id );
	}
}
