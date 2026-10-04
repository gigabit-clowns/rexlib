// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/formats/tiff/tiff_sample_type.hpp>

#include <rexlib/core/exceptions/unsupported_operation_error.hpp>

#include <cstdint>

using namespace rexlib;
using namespace rexlib::em::tiff;

namespace
{

// The values of the SampleFormat tag are restated here rather than taken
// from libtiff, so that a test written against the specification does not
// agree with the code merely by sharing its constants.
const std::uint16_t unsigned_format = 1;
const std::uint16_t signed_format = 2;
const std::uint16_t float_format = 3;
const std::uint16_t undefined_format = 4;
const std::uint16_t complex_float_format = 6;

} // anonymous namespace

TEST_CASE( "the samples of a TIFF file resolve to the data type they hold",
	"[tiff_sample_type]" )
{
	SECTION( "unsigned integers of one and two bytes have a data type" )
	{
		REQUIRE( get_data_type(8, unsigned_format) ==
			numerical_type::uint8 );
		REQUIRE( get_data_type(16, unsigned_format) ==
			numerical_type::uint16 );
	}

	SECTION( "signed integers of one and two bytes have a data type" )
	{
		REQUIRE( get_data_type(8, signed_format) == numerical_type::int8 );
		REQUIRE( get_data_type(16, signed_format) == numerical_type::int16 );
	}

	SECTION( "floating point numbers of two and four bytes have a data type" )
	{
		REQUIRE( get_data_type(16, float_format) ==
			numerical_type::float16 );
		REQUIRE( get_data_type(32, float_format) ==
			numerical_type::float32 );
	}

	SECTION( "samples narrower than a byte resolve to nothing" )
	{
		REQUIRE( get_data_type(1, unsigned_format) ==
			numerical_type::unknown );
		REQUIRE( get_data_type(4, unsigned_format) ==
			numerical_type::unknown );
	}

	SECTION( "samples wider than what is transferred resolve to nothing" )
	{
		REQUIRE( get_data_type(32, unsigned_format) ==
			numerical_type::unknown );
		REQUIRE( get_data_type(32, signed_format) ==
			numerical_type::unknown );
		REQUIRE( get_data_type(64, float_format) ==
			numerical_type::unknown );
	}

	SECTION( "a floating point number of one byte resolves to nothing" )
	{
		REQUIRE( get_data_type(8, float_format) == numerical_type::unknown );
	}

	SECTION( "any other format resolves to nothing" )
	{
		REQUIRE( get_data_type(8, 0) == numerical_type::unknown );
		REQUIRE( get_data_type(8, undefined_format) ==
			numerical_type::unknown );
		REQUIRE( get_data_type(64, complex_float_format) ==
			numerical_type::unknown );
	}
}

TEST_CASE( "a data type resolves to the samples that hold it",
	"[tiff_sample_type]" )
{
	SECTION( "the six data types with a sample are supported" )
	{
		REQUIRE( is_supported(numerical_type::int8) );
		REQUIRE( is_supported(numerical_type::uint8) );
		REQUIRE( is_supported(numerical_type::int16) );
		REQUIRE( is_supported(numerical_type::uint16) );
		REQUIRE( is_supported(numerical_type::float16) );
		REQUIRE( is_supported(numerical_type::float32) );
	}

	SECTION( "the rest are not" )
	{
		REQUIRE_FALSE( is_supported(numerical_type::unknown) );
		REQUIRE_FALSE( is_supported(numerical_type::int32) );
		REQUIRE_FALSE( is_supported(numerical_type::uint32) );
		REQUIRE_FALSE( is_supported(numerical_type::float64) );
		REQUIRE_FALSE( is_supported(numerical_type::complex_float32) );
	}

	SECTION( "a supported data type states its width" )
	{
		REQUIRE( get_bits_per_sample(numerical_type::int8) == 8 );
		REQUIRE( get_bits_per_sample(numerical_type::uint8) == 8 );
		REQUIRE( get_bits_per_sample(numerical_type::int16) == 16 );
		REQUIRE( get_bits_per_sample(numerical_type::uint16) == 16 );
		REQUIRE( get_bits_per_sample(numerical_type::float16) == 16 );
		REQUIRE( get_bits_per_sample(numerical_type::float32) == 32 );
	}

	SECTION( "a supported data type states its format" )
	{
		REQUIRE( get_sample_format(numerical_type::uint8) ==
			unsigned_format );
		REQUIRE( get_sample_format(numerical_type::uint16) ==
			unsigned_format );
		REQUIRE( get_sample_format(numerical_type::int8) == signed_format );
		REQUIRE( get_sample_format(numerical_type::int16) == signed_format );
		REQUIRE( get_sample_format(numerical_type::float16) ==
			float_format );
		REQUIRE( get_sample_format(numerical_type::float32) ==
			float_format );
	}

	SECTION( "an unsupported data type has neither" )
	{
		REQUIRE_THROWS_AS(
			get_bits_per_sample(numerical_type::float64),
			unsupported_operation_error
		);
		REQUIRE_THROWS_AS(
			get_sample_format(numerical_type::float64),
			unsupported_operation_error
		);
		REQUIRE_THROWS_AS(
			get_bits_per_sample(numerical_type::unknown),
			unsupported_operation_error
		);
		REQUIRE_THROWS_AS(
			get_sample_format(numerical_type::unknown),
			unsupported_operation_error
		);
	}

	SECTION( "what a data type resolves to resolves back to it" )
	{
		const numerical_type types[] = {
			numerical_type::int8,
			numerical_type::uint8,
			numerical_type::int16,
			numerical_type::uint16,
			numerical_type::float16,
			numerical_type::float32,
		};

		for (const auto type : types)
		{
			REQUIRE( get_data_type(
				get_bits_per_sample(type),
				get_sample_format(type)
			) == type );
		}
	}
}
