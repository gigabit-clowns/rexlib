// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/direct_image_reader_provider.hpp>

#include "fixtures/format_manager_fixture.hpp"
#include "mock/mock_image_reader.hpp"

#include <rexlib/em/image/exceptions/image_file_error.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_read_format_manager.hpp>

#include <memory>
#include <stdexcept>
#include <trompeloeil.hpp>

using namespace rexlib;
using namespace rexlib::em;

TEST_CASE( "a direct reader provider needs a format manager",
	"[direct_image_reader_provider]" )
{
	REQUIRE_THROWS_AS(
		direct_image_reader_provider(nullptr),
		std::invalid_argument
	);
}

TEST_CASE_METHOD(
	read_format_manager_fixture,
	"a direct reader provider opens a file every time it is asked",
	"[direct_image_reader_provider]"
)
{
	auto &format = add_format(backend_priority::normal);
	direct_image_reader_provider provider(get_manager());

	SECTION( "one request opens the file once" )
	{
		const auto reader = std::make_shared<mock_image_reader>();

		REQUIRE_CALL(format, open(ANY(const image_probe&)))
			.LR_WITH( _1.get_path() == "stack_0.mrcs" )
			.RETURN(reader);

		REQUIRE( provider.acquire("stack_0.mrcs") == reader );
	}

	SECTION( "the same path asked twice is opened twice" )
	{
		// It keeps nothing, which is what makes it the plain one.
		REQUIRE_CALL(format, open(ANY(const image_probe&)))
			.LR_WITH( _1.get_path() == "stack_0.mrcs" )
			.TIMES(2)
			.RETURN(std::make_shared<mock_image_reader>());

		const auto first = provider.acquire("stack_0.mrcs");
		const auto second = provider.acquire("stack_0.mrcs");

		REQUIRE( first != second );
	}

	SECTION( "distinct paths are opened separately" )
	{
		REQUIRE_CALL(format, open(ANY(const image_probe&)))
			.LR_WITH( _1.get_path() == "stack_0.mrcs" )
			.RETURN(std::make_shared<mock_image_reader>());
		REQUIRE_CALL(format, open(ANY(const image_probe&)))
			.LR_WITH( _1.get_path() == "stack_1.mrcs" )
			.RETURN(std::make_shared<mock_image_reader>());

		provider.acquire("stack_0.mrcs");
		provider.acquire("stack_1.mrcs");
	}
}

TEST_CASE( "a direct reader provider reports what the manager reports",
	"[direct_image_reader_provider]" )
{
	// A manager with no format recognizes nothing, and the provider adds
	// no opinion of its own.
	direct_image_reader_provider provider(
		std::make_shared<image_read_format_manager>()
	);

	REQUIRE_THROWS_AS(
		provider.acquire("stack_0.mrcs"),
		image_file_error
	);
}
