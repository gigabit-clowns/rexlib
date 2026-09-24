// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/formats/mrc/mrc_mode.hpp>

#include <rexlib/core/exceptions/unsupported_operation_error.hpp>

using namespace rexlib;
using namespace rexlib::em;
using namespace rexlib::em::mrc;

TEST_CASE( "an MRC mode resolves to the data type it holds",
	"[mrc_mode]" )
{
	SECTION( "every supported mode has a data type" )
	{
		REQUIRE( get_data_type(mrc_mode::int16, true) ==
			numerical_type::int16 );
		REQUIRE( get_data_type(mrc_mode::float32, true) ==
			numerical_type::float32 );
		REQUIRE( get_data_type(mrc_mode::complex_float32, true) ==
			numerical_type::complex_float32 );
		REQUIRE( get_data_type(mrc_mode::uint16, true) ==
			numerical_type::uint16 );
		REQUIRE( get_data_type(mrc_mode::float16, true) ==
			numerical_type::float16 );
	}

	SECTION( "mode 0 follows the signedness it is told" )
	{
		REQUIRE( get_data_type(mrc_mode::int8, true) ==
			numerical_type::int8 );
		REQUIRE( get_data_type(mrc_mode::int8, false) ==
			numerical_type::uint8 );
	}

	SECTION( "signedness does not reach any other mode" )
	{
		REQUIRE( get_data_type(mrc_mode::int16, false) ==
			numerical_type::int16 );
		REQUIRE( get_data_type(mrc_mode::uint16, false) ==
			numerical_type::uint16 );
	}

	SECTION( "the modes without a counterpart resolve to nothing" )
	{
		REQUIRE( get_data_type(mrc_mode::complex_int16, true) ==
			numerical_type::unknown );
		REQUIRE( get_data_type(mrc_mode::packed_4bit, true) ==
			numerical_type::unknown );
		REQUIRE( get_data_type(static_cast<mrc_mode>(77), true) ==
			numerical_type::unknown );
	}
}

TEST_CASE( "an MRC mode reports whether this format transfers it",
	"[mrc_mode]" )
{
	SECTION( "the six modes with a counterpart are supported" )
	{
		REQUIRE( is_supported(mrc_mode::int8) );
		REQUIRE( is_supported(mrc_mode::int16) );
		REQUIRE( is_supported(mrc_mode::float32) );
		REQUIRE( is_supported(mrc_mode::complex_float32) );
		REQUIRE( is_supported(mrc_mode::uint16) );
		REQUIRE( is_supported(mrc_mode::float16) );
	}

	SECTION( "the packed and complex integer modes are not" )
	{
		REQUIRE_FALSE( is_supported(mrc_mode::complex_int16) );
		REQUIRE_FALSE( is_supported(mrc_mode::packed_4bit) );
	}

	SECTION( "an unknown mode is not" )
	{
		REQUIRE_FALSE( is_supported(static_cast<mrc_mode>(-1)) );
		REQUIRE_FALSE( is_supported(static_cast<mrc_mode>(5)) );
	}
}

TEST_CASE( "an MRC mode reports the size of one of its elements",
	"[mrc_mode]" )
{
	SECTION( "each supported mode measures its data type" )
	{
		REQUIRE( get_element_size(mrc_mode::int8) == 1 );
		REQUIRE( get_element_size(mrc_mode::int16) == 2 );
		REQUIRE( get_element_size(mrc_mode::float32) == 4 );
		REQUIRE( get_element_size(mrc_mode::complex_float32) == 8 );
		REQUIRE( get_element_size(mrc_mode::uint16) == 2 );
		REQUIRE( get_element_size(mrc_mode::float16) == 2 );
	}

	SECTION( "an unsupported mode measures nothing" )
	{
		REQUIRE( get_element_size(mrc_mode::complex_int16) == 0 );
		REQUIRE( get_element_size(mrc_mode::packed_4bit) == 0 );
	}
}

TEST_CASE( "a data type resolves to the MRC mode that holds it",
	"[mrc_mode]" )
{
	SECTION( "every type the format encodes has a mode" )
	{
		REQUIRE( get_mode(numerical_type::int8) == mrc_mode::int8 );
		REQUIRE( get_mode(numerical_type::int16) == mrc_mode::int16 );
		REQUIRE( get_mode(numerical_type::float32) == mrc_mode::float32 );
		REQUIRE( get_mode(numerical_type::complex_float32) ==
			mrc_mode::complex_float32 );
		REQUIRE( get_mode(numerical_type::uint16) == mrc_mode::uint16 );
		REQUIRE( get_mode(numerical_type::float16) == mrc_mode::float16 );
	}

	SECTION( "unsigned bytes share the mode of signed ones" )
	{
		REQUIRE( get_mode(numerical_type::uint8) == mrc_mode::int8 );
	}

	SECTION( "a type the format does not encode is refused" )
	{
		REQUIRE_THROWS_AS(
			get_mode(numerical_type::int32),
			unsupported_operation_error
		);
		REQUIRE_THROWS_AS(
			get_mode(numerical_type::float64),
			unsupported_operation_error
		);
		REQUIRE_THROWS_AS(
			get_mode(numerical_type::complex_float64),
			unsupported_operation_error
		);
		REQUIRE_THROWS_AS(
			get_mode(numerical_type::boolean),
			unsupported_operation_error
		);
	}
}

TEST_CASE( "only unsigned bytes need the IMOD flag to be written",
	"[mrc_mode]" )
{
	REQUIRE( needs_imod_unsigned_flag(numerical_type::uint8) );
	REQUIRE_FALSE( needs_imod_unsigned_flag(numerical_type::int8) );
	REQUIRE_FALSE( needs_imod_unsigned_flag(numerical_type::uint16) );
	REQUIRE_FALSE( needs_imod_unsigned_flag(numerical_type::float32) );
}

TEST_CASE( "writing a data type and reading it back yields it again",
	"[mrc_mode]" )
{
	const numerical_type types[] = {
		numerical_type::int8,
		numerical_type::uint8,
		numerical_type::int16,
		numerical_type::uint16,
		numerical_type::float16,
		numerical_type::float32,
		numerical_type::complex_float32
	};

	for (const auto type : types)
	{
		const auto mode = get_mode(type);
		const auto signed_bytes = !needs_imod_unsigned_flag(type);

		REQUIRE( get_data_type(mode, signed_bytes) == type );
	}
}
