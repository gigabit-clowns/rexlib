// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/formats/tiff/tiff_sample_type.hpp>

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
const std::uint16_t complex_integer_format = 5;
const std::uint16_t complex_float_format = 6;

} // anonymous namespace

TEST_CASE( "the samples of a TIFF file resolve to the data type they hold",
	"[tiff_sample_type]" )
{
	SECTION( "unsigned integers of one to eight bytes have a data type" )
	{
		REQUIRE( get_data_type(8, unsigned_format) ==
			numerical_type::uint8 );
		REQUIRE( get_data_type(16, unsigned_format) ==
			numerical_type::uint16 );
		REQUIRE( get_data_type(32, unsigned_format) ==
			numerical_type::uint32 );
		REQUIRE( get_data_type(64, unsigned_format) ==
			numerical_type::uint64 );
	}

	SECTION( "signed integers of one to eight bytes have a data type" )
	{
		REQUIRE( get_data_type(8, signed_format) == numerical_type::int8 );
		REQUIRE( get_data_type(16, signed_format) == numerical_type::int16 );
		REQUIRE( get_data_type(32, signed_format) == numerical_type::int32 );
		REQUIRE( get_data_type(64, signed_format) == numerical_type::int64 );
	}

	SECTION( "floating point numbers of two to eight bytes have a data type" )
	{
		REQUIRE( get_data_type(16, float_format) ==
			numerical_type::float16 );
		REQUIRE( get_data_type(32, float_format) ==
			numerical_type::float32 );
		REQUIRE( get_data_type(64, float_format) ==
			numerical_type::float64 );
	}

	SECTION( "complex numbers of two floats or two doubles have a data type" )
	{
		REQUIRE( get_data_type(64, complex_float_format) ==
			numerical_type::complex_float32 );
		REQUIRE( get_data_type(128, complex_float_format) ==
			numerical_type::complex_float64 );
	}

	SECTION( "a complex number of two half precision ones resolves to nothing" )
	{
		REQUIRE( get_data_type(32, complex_float_format) ==
			numerical_type::unknown );
	}

	SECTION( "samples narrower than a byte resolve to nothing" )
	{
		REQUIRE( get_data_type(1, unsigned_format) ==
			numerical_type::unknown );
		REQUIRE( get_data_type(4, unsigned_format) ==
			numerical_type::unknown );
	}

	SECTION( "samples of a width no data type has resolve to nothing" )
	{
		REQUIRE( get_data_type(24, unsigned_format) ==
			numerical_type::unknown );
		REQUIRE( get_data_type(128, signed_format) ==
			numerical_type::unknown );
		REQUIRE( get_data_type(8, float_format) == numerical_type::unknown );
		REQUIRE( get_data_type(128, float_format) ==
			numerical_type::unknown );
	}

	SECTION( "any other format resolves to nothing" )
	{
		REQUIRE( get_data_type(8, 0) == numerical_type::unknown );
		REQUIRE( get_data_type(8, undefined_format) ==
			numerical_type::unknown );
		REQUIRE( get_data_type(32, complex_integer_format) ==
			numerical_type::unknown );
	}
}

TEST_CASE( "a data type resolves to the samples that hold it",
	"[tiff_sample_type]" )
{
	const numerical_type supported[] = {
		numerical_type::int8,
		numerical_type::uint8,
		numerical_type::int16,
		numerical_type::uint16,
		numerical_type::int32,
		numerical_type::uint32,
		numerical_type::int64,
		numerical_type::uint64,
		numerical_type::float16,
		numerical_type::float32,
		numerical_type::float64,
		numerical_type::complex_float32,
		numerical_type::complex_float64,
	};

	SECTION( "the data types with a sample are supported" )
	{
		for (const auto type : supported)
		{
			REQUIRE( is_supported(type) );
		}
	}

	SECTION( "the rest are not" )
	{
		REQUIRE_FALSE( is_supported(numerical_type::unknown) );
		REQUIRE_FALSE( is_supported(numerical_type::boolean) );
		REQUIRE_FALSE( is_supported(numerical_type::char8) );
		REQUIRE_FALSE( is_supported(numerical_type::complex_float16) );
	}

	SECTION( "a supported data type states its width" )
	{
		REQUIRE( get_bits_per_sample(numerical_type::int8) == 8 );
		REQUIRE( get_bits_per_sample(numerical_type::uint8) == 8 );
		REQUIRE( get_bits_per_sample(numerical_type::int16) == 16 );
		REQUIRE( get_bits_per_sample(numerical_type::uint16) == 16 );
		REQUIRE( get_bits_per_sample(numerical_type::int32) == 32 );
		REQUIRE( get_bits_per_sample(numerical_type::uint32) == 32 );
		REQUIRE( get_bits_per_sample(numerical_type::int64) == 64 );
		REQUIRE( get_bits_per_sample(numerical_type::uint64) == 64 );
		REQUIRE( get_bits_per_sample(numerical_type::float16) == 16 );
		REQUIRE( get_bits_per_sample(numerical_type::float32) == 32 );
		REQUIRE( get_bits_per_sample(numerical_type::float64) == 64 );
		REQUIRE( get_bits_per_sample(numerical_type::complex_float32) ==
			64 );
		REQUIRE( get_bits_per_sample(numerical_type::complex_float64) ==
			128 );
	}

	SECTION( "a supported data type states its format" )
	{
		REQUIRE( get_sample_format(numerical_type::uint8) ==
			unsigned_format );
		REQUIRE( get_sample_format(numerical_type::uint16) ==
			unsigned_format );
		REQUIRE( get_sample_format(numerical_type::uint32) ==
			unsigned_format );
		REQUIRE( get_sample_format(numerical_type::uint64) ==
			unsigned_format );
		REQUIRE( get_sample_format(numerical_type::int8) == signed_format );
		REQUIRE( get_sample_format(numerical_type::int16) == signed_format );
		REQUIRE( get_sample_format(numerical_type::int32) == signed_format );
		REQUIRE( get_sample_format(numerical_type::int64) == signed_format );
		REQUIRE( get_sample_format(numerical_type::float16) ==
			float_format );
		REQUIRE( get_sample_format(numerical_type::float32) ==
			float_format );
		REQUIRE( get_sample_format(numerical_type::float64) ==
			float_format );
		REQUIRE( get_sample_format(numerical_type::complex_float32) ==
			complex_float_format );
		REQUIRE( get_sample_format(numerical_type::complex_float64) ==
			complex_float_format );
	}

	SECTION( "an unsupported data type has neither" )
	{
		REQUIRE( get_bits_per_sample(numerical_type::complex_float16) == 0 );
		REQUIRE( get_sample_format(numerical_type::complex_float16) == 0 );
		REQUIRE( get_bits_per_sample(numerical_type::boolean) == 0 );
		REQUIRE( get_sample_format(numerical_type::boolean) == 0 );
		REQUIRE( get_bits_per_sample(numerical_type::unknown) == 0 );
		REQUIRE( get_sample_format(numerical_type::unknown) == 0 );
	}

	SECTION( "what a data type resolves to resolves back to it" )
	{
		for (const auto type : supported)
		{
			REQUIRE( get_data_type(
				get_bits_per_sample(type),
				get_sample_format(type)
			) == type );
		}
	}
}
