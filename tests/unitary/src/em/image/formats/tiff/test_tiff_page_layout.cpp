// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/formats/tiff/tiff_page_layout.hpp>

#include <cstddef>
#include <stdexcept>

using namespace rexlib;
using namespace rexlib::em::tiff;

TEST_CASE( "a page cut into strips states how its samples are held",
	"[tiff_page_layout]" )
{
	// Ten rows of seven columns, four rows to a strip: two whole strips
	// and one of the two rows left.
	const auto layout = tiff_page_layout::make_striped(
		7, 10, numerical_type::uint16, 4);

	SECTION( "it reports what it was made of" )
	{
		REQUIRE( layout.get_width() == 7 );
		REQUIRE( layout.get_height() == 10 );
		REQUIRE( layout.get_data_type() == numerical_type::uint16 );
		REQUIRE_FALSE( layout.is_tiled() );
	}

	SECTION( "a strip spans the width of the page" )
	{
		REQUIRE( layout.get_block_width() == 7 );
		REQUIRE( layout.get_block_height() == 4 );
		REQUIRE( layout.get_blocks_across() == 1 );
	}

	SECTION( "the strips cover every row" )
	{
		REQUIRE( layout.get_blocks_down() == 3 );
		REQUIRE( layout.get_block_count() == 3 );
	}

	SECTION( "the last strip holds the rows left" )
	{
		REQUIRE( layout.get_block_rows(0) == 4 );
		REQUIRE( layout.get_block_rows(1) == 4 );
		REQUIRE( layout.get_block_rows(2) == 2 );
	}

	SECTION( "a strip takes its rows in bytes" )
	{
		REQUIRE( layout.get_block_size(0) == 4 * 7 * 2 );
		REQUIRE( layout.get_block_size(2) == 2 * 7 * 2 );
	}

	SECTION( "strips that divide the page are all whole" )
	{
		const auto even = tiff_page_layout::make_striped(
			7, 8, numerical_type::uint8, 4);

		REQUIRE( even.get_block_count() == 2 );
		REQUIRE( even.get_block_rows(1) == 4 );
	}

	SECTION( "more rows to a strip than the page has stand for one strip" )
	{
		const auto single = tiff_page_layout::make_striped(
			7, 10, numerical_type::uint16, 4294967295U);

		REQUIRE( single.get_block_height() == 10 );
		REQUIRE( single.get_block_count() == 1 );
		REQUIRE( single.get_block_rows(0) == 10 );
		REQUIRE( single == tiff_page_layout::make_striped(
			7, 10, numerical_type::uint16, 10) );
	}
}

TEST_CASE( "a page cut into tiles states how its samples are held",
	"[tiff_page_layout]" )
{
	// Forty rows of fifty columns in tiles of sixteen by thirty two: four
	// tiles across and two down, the last of each reaching past the page.
	const auto layout = tiff_page_layout::make_tiled(
		50, 40, numerical_type::float32, 16, 32);

	SECTION( "it reports what it was made of" )
	{
		REQUIRE( layout.get_width() == 50 );
		REQUIRE( layout.get_height() == 40 );
		REQUIRE( layout.get_data_type() == numerical_type::float32 );
		REQUIRE( layout.is_tiled() );
		REQUIRE( layout.get_block_width() == 16 );
		REQUIRE( layout.get_block_height() == 32 );
	}

	SECTION( "the tiles cover the page" )
	{
		REQUIRE( layout.get_blocks_across() == 4 );
		REQUIRE( layout.get_blocks_down() == 2 );
		REQUIRE( layout.get_block_count() == 8 );
	}

	SECTION( "a tile is whole even past the edge of the page" )
	{
		REQUIRE( layout.get_block_rows(0) == 32 );
		REQUIRE( layout.get_block_rows(7) == 32 );
		REQUIRE( layout.get_block_size(0) == 16 * 32 * 4 );
		REQUIRE( layout.get_block_size(7) == 16 * 32 * 4 );
	}
}

TEST_CASE( "a page layout that could not hold samples is refused",
	"[tiff_page_layout]" )
{
	SECTION( "a page of no columns or no rows" )
	{
		REQUIRE_THROWS_AS(
			tiff_page_layout::make_striped(0, 4, numerical_type::uint8, 4),
			std::invalid_argument
		);
		REQUIRE_THROWS_AS(
			tiff_page_layout::make_striped(4, 0, numerical_type::uint8, 4),
			std::invalid_argument
		);
		REQUIRE_THROWS_AS(
			tiff_page_layout::make_tiled(0, 4, numerical_type::uint8, 16, 16),
			std::invalid_argument
		);
	}

	SECTION( "strips of no rows" )
	{
		REQUIRE_THROWS_AS(
			tiff_page_layout::make_striped(4, 4, numerical_type::uint8, 0),
			std::invalid_argument
		);
	}

	SECTION( "tiles of no columns or no rows" )
	{
		REQUIRE_THROWS_AS(
			tiff_page_layout::make_tiled(4, 4, numerical_type::uint8, 0, 16),
			std::invalid_argument
		);
		REQUIRE_THROWS_AS(
			tiff_page_layout::make_tiled(4, 4, numerical_type::uint8, 16, 0),
			std::invalid_argument
		);
	}

	SECTION( "an unknown data type" )
	{
		REQUIRE_THROWS_AS(
			tiff_page_layout::make_striped(4, 4, numerical_type::unknown, 4),
			std::invalid_argument
		);
	}
}

TEST_CASE( "page layouts compare by everything they state",
	"[tiff_page_layout]" )
{
	const auto layout = tiff_page_layout::make_striped(
		16, 16, numerical_type::uint8, 16);

	SECTION( "two layouts made alike are equal" )
	{
		REQUIRE( layout == tiff_page_layout::make_striped(
			16, 16, numerical_type::uint8, 16) );
		REQUIRE_FALSE( layout != tiff_page_layout::make_striped(
			16, 16, numerical_type::uint8, 16) );
	}

	SECTION( "a differing extent, data type or strip tells them apart" )
	{
		REQUIRE( layout != tiff_page_layout::make_striped(
			32, 16, numerical_type::uint8, 16) );
		REQUIRE( layout != tiff_page_layout::make_striped(
			16, 32, numerical_type::uint8, 16) );
		REQUIRE( layout != tiff_page_layout::make_striped(
			16, 16, numerical_type::int8, 16) );
		REQUIRE( layout != tiff_page_layout::make_striped(
			16, 16, numerical_type::uint8, 8) );
	}

	SECTION( "strips and tiles of one size are told apart" )
	{
		REQUIRE( layout != tiff_page_layout::make_tiled(
			16, 16, numerical_type::uint8, 16, 16) );
	}
}
