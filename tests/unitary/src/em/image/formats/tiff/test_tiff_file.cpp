// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <em/image/formats/tiff/tiff_file.hpp>

#include <rexlib/core/exceptions/unsupported_operation_error.hpp>
#include <rexlib/core/memory/byte.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/exceptions/image_file_error.hpp>
#include <rexlib/em/image/exceptions/image_format_error.hpp>

#include "../../fixtures/scoped_path.hpp"
#include "fixtures/tiff_test_file.hpp"

#include <complex>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;
using namespace rexlib::em::tiff;
using namespace rexlib::test;

namespace
{

template <typename T>
std::vector<T> read_block_of(
	tiff_file &file,
	const tiff_page_layout &layout,
	std::size_t block
)
{
	std::vector<T> samples(layout.get_block_size(block) / sizeof(T));
	file.read_block(
		block,
		make_span(as_bytes(samples.data()), samples.size() * sizeof(T))
	);

	return samples;
}

template <typename T>
span<const byte> bytes_of(const std::vector<T> &samples)
{
	return make_span(as_bytes(samples.data()), samples.size() * sizeof(T));
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

// The header of a little-endian classic file holds the offset of its first
// directory four bytes in. A directory is a count of entries of twelve bytes
// each, which begin with their tag and end with their value.
void set_first_directory_entry(
	const std::string &path,
	std::uint16_t tag,
	std::uint32_t value
)
{
	auto raw = read_file(path);
	std::uint32_t directory = 0;
	std::memcpy(&directory, raw.data() + 4, sizeof(directory));
	std::uint16_t entry_count = 0;
	std::memcpy(&entry_count, raw.data() + directory, sizeof(entry_count));

	for (std::uint16_t entry = 0; entry < entry_count; ++entry)
	{
		auto *bytes = raw.data() + directory + 2 + 12 * entry;
		std::uint16_t entry_tag = 0;
		std::memcpy(&entry_tag, bytes, sizeof(entry_tag));
		if (entry_tag == tag)
		{
			std::memcpy(bytes + 8, &value, sizeof(value));
			write_file(path, raw);
			return;
		}
	}

	throw std::runtime_error("The test file has no such directory entry.");
}

} // anonymous namespace

TEST_CASE( "a TIFF file that can not be opened says why",
	"[tiff_file]" )
{
	const scoped_path path("tiff_file_unopened.tif");

	SECTION( "a file that is not there can not be reached" )
	{
		REQUIRE_THROWS_MATCHES(
			tiff_file(path.get(), tiff_file_mode::read),
			image_file_error,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::StartsWith(path.get())
			)
		);
	}

	SECTION( "a file that holds something else is not a TIFF file" )
	{
		write_file(path.get(), std::vector<char>(64, 'x'));

		REQUIRE_THROWS_MATCHES(
			tiff_file(path.get(), tiff_file_mode::read),
			image_format_error,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::StartsWith(path.get())
			)
		);
	}

	SECTION( "a file can not be created where there is no directory" )
	{
		const auto nowhere = get_scratch_path("tiff_file_no_such_directory") +
			"/created.tif";

		REQUIRE_THROWS_AS(
			tiff_file(nowhere, tiff_file_mode::create),
			image_file_error
		);
		REQUIRE_THROWS_AS(
			tiff_file(nowhere, tiff_file_mode::create_big),
			image_file_error
		);
	}
}

TEST_CASE( "a TIFF file reports its pages and how each holds its samples",
	"[tiff_file]" )
{
	const scoped_path path("tiff_file_pages.tif");

	SECTION( "a file of one page cut into strips" )
	{
		write_striped_file<std::uint8_t>(
			path.get(), "w", 7, 10, SAMPLEFORMAT_UINT, COMPRESSION_NONE, 4,
			{counting<std::uint8_t>(70)}
		);

		tiff_file file(path.get(), tiff_file_mode::read);

		REQUIRE( file.get_page_count() == 1 );
		REQUIRE( file.get_page_layout() == tiff_page_layout::make_striped(
			7, 10, numerical_type::uint8, 4) );
	}

	SECTION( "a file of several pages" )
	{
		write_striped_file<std::int16_t>(
			path.get(), "w", 5, 3, SAMPLEFORMAT_INT, COMPRESSION_NONE, 3,
			{
				counting<std::int16_t>(15),
				counting<std::int16_t>(15),
				counting<std::int16_t>(15),
			}
		);

		tiff_file file(path.get(), tiff_file_mode::read);

		REQUIRE( file.get_page_count() == 3 );

		file.select_page(2);

		REQUIRE( file.get_page_layout() == tiff_page_layout::make_striped(
			5, 3, numerical_type::int16, 3) );
	}

	SECTION( "a page cut into tiles" )
	{
		write_tiled_file<float>(
			path.get(), 50, 40, SAMPLEFORMAT_IEEEFP, COMPRESSION_NONE, 16, 32,
			counting<float>(2000)
		);

		tiff_file file(path.get(), tiff_file_mode::read);

		REQUIRE( file.get_page_layout() == tiff_page_layout::make_tiled(
			50, 40, numerical_type::float32, 16, 32) );
	}

	SECTION( "a compressed page is laid out as any other" )
	{
		write_striped_file<std::uint16_t>(
			path.get(), "w", 6, 4, SAMPLEFORMAT_UINT, COMPRESSION_LZW, 2,
			{counting<std::uint16_t>(24)}
		);

		tiff_file file(path.get(), tiff_file_mode::read);

		REQUIRE( file.get_page_layout() == tiff_page_layout::make_striped(
			6, 4, numerical_type::uint16, 2) );
	}

	SECTION( "a BigTIFF file is opened like a classic one" )
	{
		write_striped_file<std::uint8_t>(
			path.get(), "w8", 4, 2, SAMPLEFORMAT_UINT, COMPRESSION_NONE, 2,
			{counting<std::uint8_t>(8), counting<std::uint8_t>(8)}
		);

		tiff_file file(path.get(), tiff_file_mode::read);

		REQUIRE( file.get_page_count() == 2 );
		REQUIRE( file.get_page_layout() == tiff_page_layout::make_striped(
			4, 2, numerical_type::uint8, 2) );
	}

	SECTION( "a page the file does not have can not be selected" )
	{
		write_striped_file<std::uint8_t>(
			path.get(), "w", 4, 2, SAMPLEFORMAT_UINT, COMPRESSION_NONE, 2,
			{counting<std::uint8_t>(8)}
		);

		tiff_file file(path.get(), tiff_file_mode::read);

		REQUIRE_THROWS_MATCHES(
			file.select_page(1),
			image_format_error,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::StartsWith(path.get())
			)
		);
	}
}

TEST_CASE( "a TIFF page this format can not transfer is refused",
	"[tiff_file]" )
{
	const scoped_path path("tiff_file_refused.tif");

	SECTION( "a page of several samples per pixel" )
	{
		write_rgb_file(path.get(), 4, 2);

		tiff_file file(path.get(), tiff_file_mode::read);

		REQUIRE_THROWS_MATCHES(
			file.get_page_layout(),
			image_format_error,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::ContainsSubstring("sample per pixel")
			)
		);
	}

	SECTION( "a page of samples with no data type" )
	{
		write_striped_file<std::uint8_t>(
			path.get(), "w", 4, 2, SAMPLEFORMAT_VOID, COMPRESSION_NONE, 2,
			{counting<std::uint8_t>(8)}
		);

		tiff_file file(path.get(), tiff_file_mode::read);

		REQUIRE_THROWS_MATCHES(
			file.get_page_layout(),
			image_format_error,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::ContainsSubstring("data type")
			)
		);
	}
}

TEST_CASE( "a TIFF page compressed with an unknown scheme is refused",
	"[tiff_file]" )
{
	const scoped_path path("tiff_file_unknown_scheme.tif");

	write_striped_file<std::uint8_t>(
		path.get(), "wl", 4, 2, SAMPLEFORMAT_UINT, COMPRESSION_NONE, 2,
		{counting<std::uint8_t>(8)}
	);

	// Tag 259 holds the compression scheme, and 65000 is in the range left
	// to private ones, which libtiff carries no codec for.
	set_first_directory_entry(path.get(), 259, 65000);

	tiff_file file(path.get(), tiff_file_mode::read);

	REQUIRE_THROWS_MATCHES(
		file.get_page_layout(),
		image_format_error,
		Catch::Matchers::MessageMatches(
			Catch::Matchers::ContainsSubstring("compressed")
		)
	);
}

TEST_CASE( "a TIFF file decodes the blocks of a page",
	"[tiff_file]" )
{
	const scoped_path path("tiff_file_blocks.tif");

	SECTION( "strips are decoded one by one, the last one shorter" )
	{
		const auto values = counting<std::uint8_t>(70);
		write_striped_file<std::uint8_t>(
			path.get(), "w", 7, 10, SAMPLEFORMAT_UINT, COMPRESSION_NONE, 4,
			{values}
		);

		tiff_file file(path.get(), tiff_file_mode::read);
		const auto layout = file.get_page_layout();

		REQUIRE( read_block_of<std::uint8_t>(file, layout, 0) ==
			slice(values, 0, 28) );
		REQUIRE( read_block_of<std::uint8_t>(file, layout, 1) ==
			slice(values, 28, 28) );
		REQUIRE( read_block_of<std::uint8_t>(file, layout, 2) ==
			slice(values, 56, 14) );
	}

	SECTION( "strips are decoded in any order" )
	{
		const auto values = counting<std::uint8_t>(70);
		write_striped_file<std::uint8_t>(
			path.get(), "w", 7, 10, SAMPLEFORMAT_UINT, COMPRESSION_LZW, 4,
			{values}
		);

		tiff_file file(path.get(), tiff_file_mode::read);
		const auto layout = file.get_page_layout();

		REQUIRE( read_block_of<std::uint8_t>(file, layout, 2) ==
			slice(values, 56, 14) );
		REQUIRE( read_block_of<std::uint8_t>(file, layout, 0) ==
			slice(values, 0, 28) );
	}

	SECTION( "the blocks decoded are those of the selected page" )
	{
		const auto first = counting<std::int16_t>(15);
		const auto second = counting<std::int16_t>(15, -100);
		write_striped_file<std::int16_t>(
			path.get(), "w", 5, 3, SAMPLEFORMAT_INT, COMPRESSION_LZW, 3,
			{first, second}
		);

		tiff_file file(path.get(), tiff_file_mode::read);
		const auto layout = file.get_page_layout();

		file.select_page(1);

		REQUIRE( read_block_of<std::int16_t>(file, layout, 0) == second );

		file.select_page(0);

		REQUIRE( read_block_of<std::int16_t>(file, layout, 0) == first );
	}

	SECTION( "Deflate compressed strips are decoded" )
	{
		const auto values = counting<float>(24, 0.5F);
		write_striped_file<float>(
			path.get(), "w", 6, 4, SAMPLEFORMAT_IEEEFP,
			COMPRESSION_ADOBE_DEFLATE, 4, {values}
		);

		tiff_file file(path.get(), tiff_file_mode::read);
		const auto layout = file.get_page_layout();

		REQUIRE( read_block_of<float>(file, layout, 0) == values );
	}

	SECTION( "tiles are decoded whole" )
	{
		const auto values = counting<std::uint16_t>(2000);
		write_tiled_file<std::uint16_t>(
			path.get(), 50, 40, SAMPLEFORMAT_UINT, COMPRESSION_LZW, 16, 32,
			values
		);

		tiff_file file(path.get(), tiff_file_mode::read);
		const auto layout = file.get_page_layout();

		// The tile below and to the right of the first one starts at row
		// 32 and column 16 of the page.
		const auto tile = read_block_of<std::uint16_t>(file, layout, 5);

		REQUIRE( tile.size() == 16 * 32 );
		REQUIRE( tile[0] == values[32 * 50 + 16] );
		REQUIRE( tile[15] == values[32 * 50 + 31] );
		REQUIRE( tile[7 * 16 + 3] == values[39 * 50 + 19] );
	}

	SECTION( "wide and complex samples are decoded in either byte order" )
	{
		const auto integers = counting<std::int64_t>(8, -3000000000LL);
		const std::vector<std::complex<double>> complexes = {
			{0.5, -1.5}, {0.0, 1.0}, {3.0, 4.0}, {-1.0, 0.25},
			{1.0e10, -1.0e-10}, {2.0, 2.0}, {-8.0, 0.0}, {0.0, -8.0}
		};
		const std::vector<std::complex<float>> singles = {
			{0.5F, -1.5F}, {0.0F, 1.0F}, {3.0F, 4.0F}, {-1.0F, 0.25F},
			{1.0e10F, -1.0e-10F}, {2.0F, 2.0F}, {-8.0F, 0.0F}, {0.0F, -8.0F}
		};
		const char *modes[] = {"wl", "wb"};

		for (const auto *mode : modes)
		{
			write_striped_file<std::int64_t>(
				path.get(), mode, 4, 2, SAMPLEFORMAT_INT, COMPRESSION_LZW,
				2, {integers}
			);
			{
				tiff_file file(path.get(), tiff_file_mode::read);
				const auto layout = file.get_page_layout();

				REQUIRE( layout.get_data_type() == numerical_type::int64 );
				REQUIRE( read_block_of<std::int64_t>(file, layout, 0) ==
					integers );
			}

			write_striped_file<std::complex<float>>(
				path.get(), mode, 4, 2, SAMPLEFORMAT_COMPLEXIEEEFP,
				COMPRESSION_NONE, 2, {singles}
			);
			{
				tiff_file file(path.get(), tiff_file_mode::read);
				const auto layout = file.get_page_layout();

				REQUIRE( layout.get_data_type() ==
					numerical_type::complex_float32 );
				REQUIRE( read_block_of<std::complex<float>>(
					file, layout, 0) == singles );
			}

			write_striped_file<std::complex<double>>(
				path.get(), mode, 4, 2, SAMPLEFORMAT_COMPLEXIEEEFP,
				COMPRESSION_NONE, 2, {complexes}
			);
			{
				tiff_file file(path.get(), tiff_file_mode::read);
				const auto layout = file.get_page_layout();

				REQUIRE( layout.get_data_type() ==
					numerical_type::complex_float64 );
				REQUIRE( read_block_of<std::complex<double>>(
					file, layout, 0) == complexes );
			}
		}
	}

	SECTION( "samples come in the byte order of the host" )
	{
		const auto values = counting<std::uint16_t>(8, 255);
		const char *modes[] = {"wl", "wb"};

		for (const auto *mode : modes)
		{
			write_striped_file<std::uint16_t>(
				path.get(), mode, 4, 2, SAMPLEFORMAT_UINT, COMPRESSION_NONE,
				2, {values}
			);

			tiff_file file(path.get(), tiff_file_mode::read);
			const auto layout = file.get_page_layout();

			REQUIRE( read_block_of<std::uint16_t>(file, layout, 0) ==
				values );
		}
	}

	SECTION( "a block holding fewer samples than are asked of it is refused" )
	{
		write_striped_file<std::uint8_t>(
			path.get(), "w", 7, 10, SAMPLEFORMAT_UINT, COMPRESSION_NONE, 4,
			{counting<std::uint8_t>(70)}
		);

		tiff_file file(path.get(), tiff_file_mode::read);

		// The last strip holds two rows of the four the others do.
		std::vector<byte> destination(28);

		REQUIRE_THROWS_MATCHES(
			file.read_block(
				2, make_span(destination.data(), destination.size())),
			image_format_error,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::StartsWith(path.get())
			)
		);
	}

	SECTION( "a block that lies past the end of the file can not be decoded" )
	{
		write_striped_file<std::uint8_t>(
			path.get(), "wl", 7, 10, SAMPLEFORMAT_UINT, COMPRESSION_NONE, 10,
			{counting<std::uint8_t>(70)}
		);

		// Tag 273 holds where the one strip of the page begins.
		set_first_directory_entry(path.get(), 273, 1000000);

		tiff_file file(path.get(), tiff_file_mode::read);
		const auto layout = file.get_page_layout();

		REQUIRE_THROWS_MATCHES(
			read_block_of<std::uint8_t>(file, layout, 0),
			image_format_error,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::StartsWith(path.get())
			)
		);
	}
}

TEST_CASE( "a TIFF file being created gains the pages written to it",
	"[tiff_file]" )
{
	const scoped_path path("tiff_file_created.tif");

	SECTION( "pages read back as they were written" )
	{
		const auto layout = tiff_page_layout::make_striped(
			7, 10, numerical_type::uint16, 4);
		const auto first = counting<std::uint16_t>(70);
		const auto second = counting<std::uint16_t>(70, 1000);

		{
			tiff_file file(path.get(), tiff_file_mode::create);
			file.write_page(layout, tiff_compression::lzw, bytes_of(first));
			file.write_page(layout, tiff_compression::none, bytes_of(second));
		}

		tiff_file file(path.get(), tiff_file_mode::read);

		REQUIRE( file.get_page_count() == 2 );
		REQUIRE( file.get_page_layout() == layout );
		REQUIRE( read_block_of<std::uint16_t>(file, layout, 0) ==
			slice(first, 0, 28) );
		REQUIRE( read_block_of<std::uint16_t>(file, layout, 2) ==
			slice(first, 56, 14) );

		file.select_page(1);

		REQUIRE( file.get_page_layout() == layout );
		REQUIRE( read_block_of<std::uint16_t>(file, layout, 1) ==
			slice(second, 28, 28) );
	}

	SECTION( "every data type with a sample is written" )
	{
		const auto values = counting<float>(12, -2.5F);
		const auto layout = tiff_page_layout::make_striped(
			4, 3, numerical_type::float32, 3);

		{
			tiff_file file(path.get(), tiff_file_mode::create);
			file.write_page(layout, tiff_compression::none, bytes_of(values));
		}

		tiff_file file(path.get(), tiff_file_mode::read);

		REQUIRE( file.get_page_layout() == layout );
		REQUIRE( read_block_of<float>(file, layout, 0) == values );
	}

	SECTION( "a file is created as classic or as BigTIFF" )
	{
		const auto values = counting<std::uint8_t>(8);
		const auto layout = tiff_page_layout::make_striped(
			4, 2, numerical_type::uint8, 2);

		{
			tiff_file file(path.get(), tiff_file_mode::create);
			file.write_page(layout, tiff_compression::none, bytes_of(values));
		}

		REQUIRE( read_file(path.get())[2] + read_file(path.get())[3] == 42 );

		{
			tiff_file file(path.get(), tiff_file_mode::create_big);
			file.write_page(layout, tiff_compression::none, bytes_of(values));
		}

		REQUIRE( read_file(path.get())[2] + read_file(path.get())[3] == 43 );

		tiff_file file(path.get(), tiff_file_mode::read);

		REQUIRE( read_block_of<std::uint8_t>(file, layout, 0) == values );
	}

	SECTION( "creating a file replaces the one that was there" )
	{
		write_striped_file<std::uint8_t>(
			path.get(), "w", 4, 2, SAMPLEFORMAT_UINT, COMPRESSION_NONE, 2,
			{
				counting<std::uint8_t>(8),
				counting<std::uint8_t>(8),
				counting<std::uint8_t>(8),
			}
		);

		const auto layout = tiff_page_layout::make_striped(
			3, 2, numerical_type::int8, 2);
		const auto values = counting<std::int8_t>(6, -3);

		{
			tiff_file file(path.get(), tiff_file_mode::create);
			file.write_page(layout, tiff_compression::none, bytes_of(values));
		}

		tiff_file file(path.get(), tiff_file_mode::read);

		REQUIRE( file.get_page_count() == 1 );
		REQUIRE( file.get_page_layout() == layout );
	}

	SECTION( "a page cut into tiles is not written" )
	{
		const auto values = counting<std::uint8_t>(256);
		tiff_file file(path.get(), tiff_file_mode::create);

		REQUIRE_THROWS_AS(
			file.write_page(
				tiff_page_layout::make_tiled(
					16, 16, numerical_type::uint8, 16, 16),
				tiff_compression::none,
				bytes_of(values)
			),
			std::invalid_argument
		);
	}

	SECTION( "samples that are not those of a page are not written" )
	{
		const auto values = counting<std::uint8_t>(7);
		tiff_file file(path.get(), tiff_file_mode::create);

		REQUIRE_THROWS_AS(
			file.write_page(
				tiff_page_layout::make_striped(
					4, 2, numerical_type::uint8, 2),
				tiff_compression::none,
				bytes_of(values)
			),
			std::invalid_argument
		);
	}

	SECTION( "a data type with no sample is not written" )
	{
		const auto values = counting<std::uint8_t>(8);
		tiff_file file(path.get(), tiff_file_mode::create);

		REQUIRE_THROWS_AS(
			file.write_page(
				tiff_page_layout::make_striped(
					4, 2, numerical_type::boolean, 2),
				tiff_compression::none,
				bytes_of(values)
			),
			unsupported_operation_error
		);
	}
}
