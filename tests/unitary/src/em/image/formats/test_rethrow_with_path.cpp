// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <em/image/formats/rethrow_with_path.hpp>

#include <rexlib/core/exceptions/unsupported_capability_error.hpp>
#include <rexlib/core/exceptions/unsupported_operation_error.hpp>
#include <rexlib/em/image/exceptions/image_file_error.hpp>
#include <rexlib/em/image/exceptions/image_format_error.hpp>

#include <stdexcept>
#include <string>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

template <typename Error>
void throw_and_rethrow(const std::string &message)
{
	try
	{
		throw Error(message);
	}
	catch (...)
	{
		rethrow_with_path("stack.mrcs");
	}
}

} // anonymous namespace

TEST_CASE(
	"rethrow_with_path keeps the type and puts the path first",
	"[rethrow_with_path]"
)
{
	const auto named = Catch::Matchers::Message("stack.mrcs: It failed.");

	SECTION( "an image_file_error" )
	{
		REQUIRE_THROWS_MATCHES(
			throw_and_rethrow<image_file_error>("It failed."),
			image_file_error,
			named
		);
	}

	SECTION( "an image_format_error" )
	{
		REQUIRE_THROWS_MATCHES(
			throw_and_rethrow<image_format_error>("It failed."),
			image_format_error,
			named
		);
	}

	SECTION( "an unsupported_operation_error" )
	{
		REQUIRE_THROWS_MATCHES(
			throw_and_rethrow<unsupported_operation_error>("It failed."),
			unsupported_operation_error,
			named
		);
	}

	SECTION( "an unsupported_capability_error" )
	{
		REQUIRE_THROWS_MATCHES(
			throw_and_rethrow<unsupported_capability_error>("It failed."),
			unsupported_capability_error,
			named
		);
	}

	SECTION( "a std::out_of_range" )
	{
		REQUIRE_THROWS_MATCHES(
			throw_and_rethrow<std::out_of_range>("It failed."),
			std::out_of_range,
			named
		);
	}

	SECTION( "a std::invalid_argument" )
	{
		REQUIRE_THROWS_MATCHES(
			throw_and_rethrow<std::invalid_argument>("It failed."),
			std::invalid_argument,
			named
		);
	}
}

TEST_CASE(
	"rethrow_with_path leaves any other exception as it was",
	"[rethrow_with_path]"
)
{
	REQUIRE_THROWS_MATCHES(
		throw_and_rethrow<std::runtime_error>("It failed."),
		std::runtime_error,
		Catch::Matchers::Message("It failed.")
	);
}
