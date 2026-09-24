// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_read_format_manager.hpp>

#include "fixtures/format_manager_fixture.hpp"
#include "mock/mock_image_reader.hpp"
#include "mock/mock_image_read_format.hpp"

#include <rexlib/core/exceptions/invalid_operation_error.hpp>
#include <rexlib/em/image/image_probe.hpp>

#include <memory>
#include <string>
#include <trompeloeil.hpp>

using namespace rexlib;
using namespace rexlib::em;

TEST_CASE( "an empty read manager recognizes nothing",
	"[image_read_format_manager]" )
{
	const image_read_format_manager manager;

	SECTION( "no format claims a file" )
	{
		REQUIRE( manager.get_most_suitable_format(
			image_probe("absent.mrc")) == nullptr );
	}

	SECTION( "opening reports that nothing is suitable" )
	{
		REQUIRE_THROWS_AS(
			manager.open("absent.mrc"),
			invalid_operation_error
		);
	}
}

TEST_CASE_METHOD(
	read_format_manager_fixture,
	"the read manager picks the most suitable format",
	"[image_read_format_manager]"
)
{
	const auto &manager = *get_manager();
	const image_probe probe("absent.mrc");

	SECTION( "the only supporting format is chosen" )
	{
		const auto &only = add_format(backend_priority::normal);

		REQUIRE( manager.get_most_suitable_format(probe) == &only );
	}

	SECTION( "the highest priority wins" )
	{
		add_format(backend_priority::fallback);
		const auto &optimal = add_format(backend_priority::optimal);
		add_format(backend_priority::normal);

		REQUIRE( manager.get_most_suitable_format(probe) == &optimal );
	}

	SECTION( "a format reporting unsupported is never chosen" )
	{
		add_format(backend_priority::unsupported);
		const auto &accepts = add_format(backend_priority::fallback);

		REQUIRE( manager.get_most_suitable_format(probe) == &accepts );
	}

	SECTION( "every format declining leaves nothing suitable" )
	{
		add_format(backend_priority::unsupported);
		add_format(backend_priority::unsupported);

		REQUIRE( manager.get_most_suitable_format(probe) == nullptr );
		REQUIRE_THROWS_AS(
			manager.open("absent.mrc"),
			invalid_operation_error
		);
	}

	SECTION( "the chosen format opens the reader" )
	{
		auto &only = add_format(backend_priority::normal);
		const auto reader = std::make_shared<mock_image_reader>();

		REQUIRE_CALL(only, open(ANY(const image_probe&)))
			.LR_WITH( _1.get_path() == "absent.mrc" )
			.RETURN(reader);

		REQUIRE( manager.open("absent.mrc") == reader );
	}
}

TEST_CASE( "the read manager refuses a null format",
	"[image_read_format_manager]" )
{
	image_read_format_manager manager;

	REQUIRE_FALSE( manager.register_format(nullptr) );
	REQUIRE( manager.register_format(
		std::make_unique<mock_image_read_format>()) );
}

TEST_CASE( "the read manager consults every registered format",
	"[image_read_format_manager]" )
{
	image_read_format_manager manager;

	auto first = std::make_unique<mock_image_read_format>();
	auto second = std::make_unique<mock_image_read_format>();

	REQUIRE_CALL(*first, get_suitability(ANY(const image_probe&)))
		.RETURN(backend_priority::unsupported);
	REQUIRE_CALL(*second, get_suitability(ANY(const image_probe&)))
		.RETURN(backend_priority::normal);
	ALLOW_CALL(*second, get_name()).RETURN(std::string("second"));

	const auto *expected = second.get();
	manager.register_format(std::move(first));
	manager.register_format(std::move(second));

	const auto *chosen = manager.get_most_suitable_format(
		image_probe("absent.mrc"));

	REQUIRE( chosen == expected );
}
