// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rexlib/core/ndarray/access_hazard_tracker.hpp>

#include "../hardware/mock/mock_command_timeline.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace rexlib;

namespace
{

std::vector<std::size_t> ids_of(const std::vector<command_token> &tokens)
{
	std::vector<std::size_t> result;
	for (const auto &token : tokens)
	{
		result.push_back(token.get_id());
	}
	return result;
}

std::vector<std::size_t> collect_ids(
	const access_hazard_tracker &tracker,
	access_flags access
)
{
	std::vector<command_token> tokens;
	tracker.collect(access, tokens);
	return ids_of(tokens);
}

} // anonymous namespace

TEST_CASE(
	"access_hazard_tracker starts with nothing to wait for",
	"[access_hazard_tracker]"
)
{
	const auto access = GENERATE(read_only, write_only, read_write);
	const access_hazard_tracker tracker;

	CHECK( collect_ids(tracker, access).empty() );
	REQUIRE_NOTHROW( tracker.wait(access) );
}

TEST_CASE(
	"access_hazard_tracker makes a read wait for the last write",
	"[access_hazard_tracker]"
)
{
	const auto timeline = std::make_shared<mock_command_timeline>();
	access_hazard_tracker tracker;

	tracker.add(command_token(timeline, 1), write_only);

	std::vector<command_token> tokens;
	tracker.collect(read_only, tokens);

	REQUIRE( tokens.size() == 1 );
	CHECK( tokens[0].get_timeline() == timeline );
	CHECK( tokens[0].get_id() == 1u );
}

TEST_CASE(
	"access_hazard_tracker does not make a read wait for other reads",
	"[access_hazard_tracker]"
)
{
	const auto timeline = std::make_shared<mock_command_timeline>();
	access_hazard_tracker tracker;

	ALLOW_CALL(*timeline, is_complete(trompeloeil::_))
		.RETURN(false);

	tracker.add(command_token(timeline, 1), read_only);
	tracker.add(command_token(timeline, 2), read_only);

	CHECK( collect_ids(tracker, read_only).empty() );
}

TEST_CASE(
	"access_hazard_tracker makes a write wait for the last write and for "
	"every read since",
	"[access_hazard_tracker]"
)
{
	const auto access = GENERATE(write_only, read_write);
	const auto timeline = std::make_shared<mock_command_timeline>();
	access_hazard_tracker tracker;

	ALLOW_CALL(*timeline, is_complete(trompeloeil::_))
		.RETURN(false);

	tracker.add(command_token(timeline, 1), write_only);
	tracker.add(command_token(timeline, 2), read_only);
	tracker.add(command_token(timeline, 3), read_only);

	const std::vector<std::size_t> expected = { 1, 2, 3 };
	CHECK( collect_ids(tracker, access) == expected );
}

TEST_CASE(
	"access_hazard_tracker forgets what a write was recorded after",
	"[access_hazard_tracker]"
)
{
	const auto write = GENERATE(write_only, read_write);
	const auto access = GENERATE(read_only, write_only);
	const auto timeline = std::make_shared<mock_command_timeline>();
	access_hazard_tracker tracker;

	tracker.add(command_token(timeline, 1), write_only);
	tracker.add(command_token(timeline, 2), read_only);
	tracker.add(command_token(timeline, 3), write);

	const std::vector<std::size_t> expected = { 3 };
	CHECK( collect_ids(tracker, access) == expected );
}

TEST_CASE(
	"access_hazard_tracker does not keep a token that stands for no command",
	"[access_hazard_tracker]"
)
{
	const auto timeline = std::make_shared<mock_command_timeline>();
	access_hazard_tracker tracker;

	SECTION("as a read")
	{
		tracker.add(command_token(), read_only);
		CHECK( collect_ids(tracker, write_only).empty() );
	}

	SECTION("as a write, which still replaces what was recorded")
	{
		tracker.add(command_token(timeline, 1), write_only);
		tracker.add(command_token(timeline, 2), read_only);
		tracker.add(command_token(), write_only);
		CHECK( collect_ids(tracker, write_only).empty() );
	}
}

TEST_CASE(
	"access_hazard_tracker::collect appends to the tokens it is given",
	"[access_hazard_tracker]"
)
{
	const auto timeline = std::make_shared<mock_command_timeline>();
	access_hazard_tracker tracker;
	tracker.add(command_token(timeline, 2), write_only);

	std::vector<command_token> tokens = { command_token(timeline, 1) };
	tracker.collect(read_only, tokens);

	const std::vector<std::size_t> expected = { 1, 2 };
	CHECK( ids_of(tokens) == expected );
}

TEST_CASE(
	"access_hazard_tracker::wait waits for the tokens of the access",
	"[access_hazard_tracker]"
)
{
	const auto timeline = std::make_shared<mock_command_timeline>();
	access_hazard_tracker tracker;

	tracker.add(command_token(timeline, 1), write_only);
	tracker.add(command_token(timeline, 2), read_only);

	SECTION("a read waits for the write")
	{
		REQUIRE_CALL(*timeline, wait(1u));
		tracker.wait(read_only);
	}

	SECTION("a write waits for the write and for the reads")
	{
		REQUIRE_CALL(*timeline, wait(1u));
		REQUIRE_CALL(*timeline, wait(2u));
		tracker.wait(write_only);
	}
}

TEST_CASE(
	"access_hazard_tracker drops complete reads as reads accumulate",
	"[access_hazard_tracker]"
)
{
	const std::size_t count = 64;
	const auto timeline = std::make_shared<mock_command_timeline>();
	access_hazard_tracker tracker;

	// The reads with an even id are complete, the others are not.
	ALLOW_CALL(*timeline, is_complete(trompeloeil::_))
		.RETURN(_1 % 2 == 0);

	for (std::size_t id = 1; id <= count; ++id)
	{
		tracker.add(command_token(timeline, id), read_only);
	}

	const auto ids = collect_ids(tracker, write_only);

	CHECK( ids.size() < count );
	for (std::size_t id = 1; id <= count; id += 2)
	{
		CAPTURE( id );
		CHECK( std::find(ids.cbegin(), ids.cend(), id) != ids.cend() );
	}
}

TEST_CASE(
	"access_hazard_tracker rejects an access that neither reads nor writes",
	"[access_hazard_tracker]"
)
{
	access_hazard_tracker tracker;
	std::vector<command_token> tokens;

	CHECK_THROWS_AS(
		tracker.add(command_token(), access_flags()),
		std::invalid_argument
	);
	CHECK_THROWS_AS(
		tracker.collect(access_flags(), tokens),
		std::invalid_argument
	);
	CHECK_THROWS_AS(
		tracker.wait(access_flags()),
		std::invalid_argument
	);
}
