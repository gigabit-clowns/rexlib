// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <em/image/formats/tiff/tiff_page_decoder.hpp>

#include <rexlib/em/image/exceptions/image_file_error.hpp>
#include <rexlib/em/image/exceptions/image_format_error.hpp>

#include "../../fixtures/scoped_path.hpp"
#include "fixtures/tiff_test_file.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;
using namespace rexlib::em::tiff;
using namespace rexlib::test;

namespace
{

// The samples of some rows of a page, taken out of what a decoder returned
// for the whole of it.
template <typename T>
std::vector<T> rows_of(
	const byte *page,
	std::size_t width,
	std::size_t first_row,
	std::size_t row_count
)
{
	std::vector<T> samples(row_count * width);
	std::memcpy(
		samples.data(),
		page + first_row * width * sizeof(T),
		samples.size() * sizeof(T)
	);

	return samples;
}

template <typename T>
std::vector<T> slice(
	const std::vector<T> &values,
	std::size_t first,
	std::size_t count
)
{
	return std::vector<T>(
		values.begin() + static_cast<std::ptrdiff_t>(first),
		values.begin() + static_cast<std::ptrdiff_t>(first + count)
	);
}

} // anonymous namespace

TEST_CASE( "a page decoder reports what every page of its file is",
	"[tiff_page_decoder]" )
{
	const scoped_path path("tiff_page_decoder_reported.tif");

	SECTION( "a file of one page" )
	{
		write_striped_file<std::uint8_t>(
			path.get(), "w", 7, 10, SAMPLEFORMAT_UINT, COMPRESSION_NONE, 4,
			{counting<std::uint8_t>(70)}
		);

		const tiff_page_decoder decoder(path.get());

		REQUIRE( decoder.get_page_count() == 1 );
		REQUIRE( decoder.get_width() == 7 );
		REQUIRE( decoder.get_height() == 10 );
		REQUIRE( decoder.get_data_type() == numerical_type::uint8 );
	}

	SECTION( "a file of several pages" )
	{
		write_striped_file<float>(
			path.get(), "w", 5, 3, SAMPLEFORMAT_IEEEFP, COMPRESSION_LZW, 3,
			{counting<float>(15), counting<float>(15), counting<float>(15)}
		);

		const tiff_page_decoder decoder(path.get());

		REQUIRE( decoder.get_page_count() == 3 );
		REQUIRE( decoder.get_width() == 5 );
		REQUIRE( decoder.get_height() == 3 );
		REQUIRE( decoder.get_data_type() == numerical_type::float32 );
	}
}

TEST_CASE( "a page decoder refuses a file it can not take for a stack",
	"[tiff_page_decoder]" )
{
	const scoped_path path("tiff_page_decoder_refused.tif");

	SECTION( "one that is not there" )
	{
		REQUIRE_THROWS_AS( tiff_page_decoder(path.get()), image_file_error );
	}

	SECTION( "one whose first page can not be transferred" )
	{
		write_rgb_file(path.get(), 4, 2);

		REQUIRE_THROWS_AS( tiff_page_decoder(path.get()), image_format_error );
	}

	SECTION( "one whose pages differ in size" )
	{
		write_striped_file<std::uint8_t>(
			path.get(), "w", 4, 2, SAMPLEFORMAT_UINT, COMPRESSION_NONE, 2,
			{counting<std::uint8_t>(8)}
		);
		append_striped_page<std::uint8_t>(
			path.get(), 4, 3, SAMPLEFORMAT_UINT, COMPRESSION_NONE, 3,
			counting<std::uint8_t>(12)
		);

		REQUIRE_THROWS_MATCHES(
			tiff_page_decoder(path.get()),
			image_format_error,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::StartsWith(path.get()) &&
				Catch::Matchers::ContainsSubstring("differ")
			)
		);
	}

	SECTION( "one whose pages differ in data type" )
	{
		write_striped_file<std::uint8_t>(
			path.get(), "w", 4, 2, SAMPLEFORMAT_UINT, COMPRESSION_NONE, 2,
			{counting<std::uint8_t>(8)}
		);
		append_striped_page<std::int8_t>(
			path.get(), 4, 2, SAMPLEFORMAT_INT, COMPRESSION_NONE, 2,
			counting<std::int8_t>(8)
		);

		REQUIRE_THROWS_AS( tiff_page_decoder(path.get()), image_format_error );
	}

	SECTION( "one whose later page can not be transferred" )
	{
		write_striped_file<std::uint8_t>(
			path.get(), "w", 4, 2, SAMPLEFORMAT_UINT, COMPRESSION_NONE, 2,
			{counting<std::uint8_t>(8)}
		);
		append_striped_page<std::uint8_t>(
			path.get(), 4, 2, SAMPLEFORMAT_VOID, COMPRESSION_NONE, 2,
			counting<std::uint8_t>(8)
		);

		REQUIRE_THROWS_AS( tiff_page_decoder(path.get()), image_format_error );
	}
}

TEST_CASE( "a page decoder decodes the rows asked of it",
	"[tiff_page_decoder]" )
{
	const scoped_path path("tiff_page_decoder_rows.tif");

	SECTION( "the whole of a page cut into strips" )
	{
		const auto values = counting<std::uint16_t>(70);
		write_striped_file<std::uint16_t>(
			path.get(), "w", 7, 10, SAMPLEFORMAT_UINT, COMPRESSION_LZW, 4,
			{values}
		);

		tiff_page_decoder decoder(path.get());

		REQUIRE( rows_of<std::uint16_t>(decoder.decode(0, 0, 10), 7, 0, 10) ==
			values );
	}

	SECTION( "rows within one strip" )
	{
		const auto values = counting<std::uint16_t>(70);
		write_striped_file<std::uint16_t>(
			path.get(), "w", 7, 10, SAMPLEFORMAT_UINT, COMPRESSION_LZW, 4,
			{values}
		);

		tiff_page_decoder decoder(path.get());

		REQUIRE( rows_of<std::uint16_t>(decoder.decode(0, 5, 2), 7, 5, 2) ==
			slice(values, 35, 14) );
	}

	SECTION( "rows across strips, the last one shorter" )
	{
		const auto values = counting<std::uint16_t>(70);
		write_striped_file<std::uint16_t>(
			path.get(), "w", 7, 10, SAMPLEFORMAT_UINT, COMPRESSION_LZW, 4,
			{values}
		);

		tiff_page_decoder decoder(path.get());

		REQUIRE( rows_of<std::uint16_t>(decoder.decode(0, 7, 3), 7, 7, 3) ==
			slice(values, 49, 21) );
	}

	SECTION( "the whole of a page cut into tiles that reach past it" )
	{
		const auto values = counting<std::uint16_t>(2000);
		write_tiled_file<std::uint16_t>(
			path.get(), 50, 40, SAMPLEFORMAT_UINT, COMPRESSION_LZW, 16, 32,
			values
		);

		tiff_page_decoder decoder(path.get());

		REQUIRE( rows_of<std::uint16_t>(decoder.decode(0, 0, 40), 50, 0, 40) ==
			values );
	}

	SECTION( "rows within one row of tiles" )
	{
		const auto values = counting<std::uint16_t>(2000);
		write_tiled_file<std::uint16_t>(
			path.get(), 50, 40, SAMPLEFORMAT_UINT, COMPRESSION_NONE, 16, 32,
			values
		);

		tiff_page_decoder decoder(path.get());

		REQUIRE( rows_of<std::uint16_t>(decoder.decode(0, 33, 4), 50, 33, 4) ==
			slice(values, 33 * 50, 4 * 50) );
	}

	SECTION( "no row at all" )
	{
		write_striped_file<std::uint8_t>(
			path.get(), "w", 7, 10, SAMPLEFORMAT_UINT, COMPRESSION_NONE, 4,
			{counting<std::uint8_t>(70)}
		);

		tiff_page_decoder decoder(path.get());

		REQUIRE( decoder.decode(0, 3, 0) != nullptr );
	}
}

TEST_CASE( "a page decoder moves between the pages of its file",
	"[tiff_page_decoder]" )
{
	const scoped_path path("tiff_page_decoder_pages.tif");
	const auto first = counting<std::int16_t>(15);
	const auto second = counting<std::int16_t>(15, -100);
	const auto third = counting<std::int16_t>(15, 1000);
	write_striped_file<std::int16_t>(
		path.get(), "w", 5, 3, SAMPLEFORMAT_INT, COMPRESSION_LZW, 1,
		{first, second, third}
	);

	tiff_page_decoder decoder(path.get());

	SECTION( "each page decodes to its own samples" )
	{
		REQUIRE( rows_of<std::int16_t>(decoder.decode(2, 0, 3), 5, 0, 3) ==
			third );
		REQUIRE( rows_of<std::int16_t>(decoder.decode(0, 0, 3), 5, 0, 3) ==
			first );
		REQUIRE( rows_of<std::int16_t>(decoder.decode(1, 0, 3), 5, 0, 3) ==
			second );
	}

	SECTION( "rows asked of one page in turns add up" )
	{
		decoder.decode(1, 0, 1);
		decoder.decode(1, 2, 1);

		REQUIRE( rows_of<std::int16_t>(decoder.decode(1, 1, 1), 5, 0, 3) ==
			second );
	}

	SECTION( "a page asked for again after another is decoded afresh" )
	{
		decoder.decode(1, 0, 3);
		decoder.decode(2, 0, 1);

		REQUIRE( rows_of<std::int16_t>(decoder.decode(1, 2, 1), 5, 2, 1) ==
			slice(second, 10, 5) );
	}

	SECTION( "rows kept are not read from the file again" )
	{
		decoder.decode(1, 0, 3);

		// With the file gone from under the decoder's feet, anything read
		// now could only come from what it kept.
		write_file(path.get(), std::vector<char>(8, '\0'));

		REQUIRE( rows_of<std::int16_t>(decoder.decode(1, 0, 3), 5, 0, 3) ==
			second );
	}
}

TEST_CASE( "a page decoder decodes pages that differ in how they are cut",
	"[tiff_page_decoder]" )
{
	const scoped_path path("tiff_page_decoder_mixed.tif");
	const auto first = counting<std::uint8_t>(70);
	const auto second = counting<std::uint8_t>(70, 100);
	write_striped_file<std::uint8_t>(
		path.get(), "w", 7, 10, SAMPLEFORMAT_UINT, COMPRESSION_NONE, 4,
		{first}
	);
	append_striped_page<std::uint8_t>(
		path.get(), 7, 10, SAMPLEFORMAT_UINT, COMPRESSION_LZW, 3, second
	);

	tiff_page_decoder decoder(path.get());

	REQUIRE( decoder.get_page_count() == 2 );
	REQUIRE( rows_of<std::uint8_t>(decoder.decode(1, 0, 10), 7, 0, 10) ==
		second );
	REQUIRE( rows_of<std::uint8_t>(decoder.decode(0, 0, 10), 7, 0, 10) ==
		first );
}
