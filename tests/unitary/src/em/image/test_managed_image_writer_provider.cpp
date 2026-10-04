// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <rexlib/em/image/managed_image_writer_provider.hpp>

#include "fixtures/format_manager_fixture.hpp"
#include "mock/mock_image_writer.hpp"

#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_probe.hpp>

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <trompeloeil.hpp>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

const std::vector<std::size_t> stack_extents = {4, 3, 5};
const image_descriptor stack_descriptor(
	make_span(stack_extents),
	2,
	numerical_type::int16
);

void declare_stack(
	managed_image_writer_provider &provider,
	std::string path
)
{
	provider.declare(std::move(path), stack_descriptor, image_metadata());
}

} // anonymous namespace

TEST_CASE( "a managed writer provider needs a format manager",
	"[managed_image_writer_provider]" )
{
	REQUIRE_THROWS_AS(
		managed_image_writer_provider(nullptr),
		std::invalid_argument
	);
}

TEST_CASE_METHOD(
	write_format_manager_fixture,
	"a managed writer provider serves only what was declared",
	"[managed_image_writer_provider]"
)
{
	// No expectation on open: creating any file would violate.
	add_format(backend_priority::normal);
	managed_image_writer_provider provider(get_manager());

	SECTION( "it starts serving nothing" )
	{
		REQUIRE( provider.get_file_count() == 0 );
		REQUIRE_THROWS_MATCHES(
			provider.acquire("stack_0.mrcs"),
			std::out_of_range,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::StartsWith("stack_0.mrcs: ")
			)
		);
	}

	SECTION( "declaring costs no file" )
	{
		declare_stack(provider, "stack_0.mrcs");

		REQUIRE( provider.get_file_count() == 1 );
	}

	SECTION( "a declared path is refused a second time" )
	{
		// Replacing it would strand whatever had been written to the file
		// it names, so the caller has to close it and say so.
		declare_stack(provider, "stack_0.mrcs");

		REQUIRE_THROWS_MATCHES(
			declare_stack(provider, "stack_0.mrcs"),
			std::logic_error,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::StartsWith("stack_0.mrcs: ")
			)
		);
		REQUIRE( provider.get_file_count() == 1 );
	}
}

TEST_CASE_METHOD(
	write_format_manager_fixture,
	"a managed writer provider creates a file once",
	"[managed_image_writer_provider]"
)
{
	auto &format = add_format(backend_priority::normal);
	managed_image_writer_provider provider(get_manager());
	declare_stack(provider, "stack_0.mrcs");
	const auto writer = std::make_shared<mock_image_writer>();

	// Once, with what was declared: creating it again would replace it.
	REQUIRE_CALL(format, open(trompeloeil::_, trompeloeil::_, trompeloeil::_))
		.LR_WITH( _1.get_path() == "stack_0.mrcs" && _2 == stack_descriptor )
		.RETURN(writer);

	SECTION( "the first acquire creates it" )
	{
		REQUIRE( provider.acquire("stack_0.mrcs") == writer );
	}

	SECTION( "every later acquire yields the same writer" )
	{
		provider.acquire("stack_0.mrcs");

		REQUIRE( provider.acquire("stack_0.mrcs") == writer );
	}
}

TEST_CASE_METHOD(
	write_format_manager_fixture,
	"closing a file finishes it",
	"[managed_image_writer_provider]"
)
{
	auto &format = add_format(backend_priority::normal);
	managed_image_writer_provider provider(get_manager());
	declare_stack(provider, "stack_0.mrcs");
	const auto writer = std::make_shared<mock_image_writer>();

	SECTION( "closing what was never declared is refused" )
	{
		REQUIRE_THROWS_MATCHES(
			provider.close("absent.mrcs"),
			std::out_of_range,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::StartsWith("absent.mrcs: ")
			)
		);
	}

	SECTION( "close flushes the writer it drops" )
	{
		REQUIRE_CALL(
			format,
			open(trompeloeil::_, trompeloeil::_, trompeloeil::_)
		)
			.RETURN(writer);
		REQUIRE_CALL(*writer, flush());

		provider.acquire("stack_0.mrcs");
		provider.close("stack_0.mrcs");
	}

	SECTION( "closing forgets the declaration, not just the handle" )
	{
		// A handle-only close would let the next acquire replace the file
		// and strand everything already written to it.
		REQUIRE_CALL(
			format,
			open(trompeloeil::_, trompeloeil::_, trompeloeil::_)
		)
			.RETURN(writer);
		ALLOW_CALL(*writer, flush());

		provider.acquire("stack_0.mrcs");
		provider.close("stack_0.mrcs");

		REQUIRE( provider.get_file_count() == 0 );
		REQUIRE_THROWS_AS(
			provider.acquire("stack_0.mrcs"),
			std::out_of_range
		);
	}

	SECTION( "closing a file never acquired creates nothing" )
	{
		// No expectation on open: creating the file would violate.
		provider.close("stack_0.mrcs");

		REQUIRE( provider.get_file_count() == 0 );
	}

	SECTION( "declaring again after a close creates the file afresh" )
	{
		const auto again = std::make_shared<mock_image_writer>();
		trompeloeil::sequence order;

		REQUIRE_CALL(
			format,
			open(trompeloeil::_, trompeloeil::_, trompeloeil::_)
		)
			.IN_SEQUENCE(order)
			.RETURN(writer);
		REQUIRE_CALL(
			format,
			open(trompeloeil::_, trompeloeil::_, trompeloeil::_)
		)
			.IN_SEQUENCE(order)
			.RETURN(again);
		ALLOW_CALL(*writer, flush());

		provider.acquire("stack_0.mrcs");
		provider.close("stack_0.mrcs");
		declare_stack(provider, "stack_0.mrcs");

		REQUIRE( provider.acquire("stack_0.mrcs") == again );
	}
}

TEST_CASE_METHOD(
	write_format_manager_fixture,
	"a managed writer provider flushes what it opened and no more",
	"[managed_image_writer_provider]"
)
{
	auto &format = add_format(backend_priority::normal);
	managed_image_writer_provider provider(get_manager());
	declare_stack(provider, "stack_0.mrcs");
	declare_stack(provider, "stack_1.mrcs");
	declare_stack(provider, "stack_2.mrcs");

	// Nothing lets stack_2.mrcs be opened, so creating it would violate.
	const auto zero = std::make_shared<mock_image_writer>();
	const auto one = std::make_shared<mock_image_writer>();
	REQUIRE_CALL(format, open(trompeloeil::_, trompeloeil::_, trompeloeil::_))
		.LR_WITH( _1.get_path() == "stack_0.mrcs" )
		.RETURN(zero);
	REQUIRE_CALL(format, open(trompeloeil::_, trompeloeil::_, trompeloeil::_))
		.LR_WITH( _1.get_path() == "stack_1.mrcs" )
		.RETURN(one);

	provider.acquire("stack_0.mrcs");
	provider.acquire("stack_1.mrcs");

	SECTION( "every writer it created is flushed" )
	{
		REQUIRE_CALL(*zero, flush());
		REQUIRE_CALL(*one, flush());

		provider.flush();
	}

	SECTION( "a declared file that was never acquired is not created" )
	{
		// Not even by a flush, which reaches only what it opened.
		ALLOW_CALL(*zero, flush());
		ALLOW_CALL(*one, flush());

		provider.flush();

		REQUIRE( provider.get_file_count() == 3 );
	}
}
