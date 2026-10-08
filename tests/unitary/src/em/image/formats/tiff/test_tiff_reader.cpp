// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/formats/tiff/tiff_reader.hpp>

#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/em/image/exceptions/image_file_error.hpp>
#include <rexlib/em/image/exceptions/image_format_error.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>

#include "../../fixtures/scoped_path.hpp"
#include "fixtures/host_array.hpp"
#include "fixtures/tiff_test_file.hpp"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;
using namespace rexlib::em::tiff;
using namespace rexlib::test;

namespace
{

// A stack of three pages of four rows and five columns, the samples of the
// whole of it counting up from zero, cut into strips of three rows.
const std::vector<std::size_t> stack_extents = {3, 4, 5};

std::vector<std::uint16_t> write_stack(
	const std::string &path,
	std::uint16_t compression
)
{
	const auto values = counting<std::uint16_t>(60);
	write_striped_file<std::uint16_t>(
		path, "w", 5, 4, SAMPLEFORMAT_UINT, compression, 3,
		{
			std::vector<std::uint16_t>(values.begin(), values.begin() + 20),
			std::vector<std::uint16_t>(
				values.begin() + 20, values.begin() + 40),
			std::vector<std::uint16_t>(values.begin() + 40, values.end()),
		}
	);

	return values;
}

template <typename T>
std::vector<T> read_all(
	const tiff_reader &reader,
	const std::vector<std::size_t> &extents,
	numerical_type data_type
)
{
	auto destination = make_host_array<T>(extents, data_type);
	reader.read(array_ref(destination), whole_of(extents));

	return values_of<T>(destination, extents);
}

} // anonymous namespace

TEST_CASE( "a TIFF reader describes its file by its pages",
	"[tiff_reader]" )
{
	const scoped_path path("tiff_reader_described.tif");

	SECTION( "a file of one page is an image" )
	{
		write_striped_file<std::uint8_t>(
			path.get(), "w", 7, 10, SAMPLEFORMAT_UINT, COMPRESSION_NONE, 4,
			{counting<std::uint8_t>(70)}
		);

		const tiff_reader reader(path.get());

		const std::vector<std::size_t> extents = {10, 7};

		REQUIRE( reader.get_descriptor() == image_descriptor(
			make_span(extents), 2, numerical_type::uint8) );
	}

	SECTION( "a file of several pages is a stack of images" )
	{
		write_stack(path.get(), COMPRESSION_NONE);

		const tiff_reader reader(path.get());

		REQUIRE( reader.get_descriptor() == image_descriptor(
			make_span(stack_extents), 2, numerical_type::uint16) );
	}
}

TEST_CASE( "a TIFF reader reads the regions asked of it",
	"[tiff_reader]" )
{
	const scoped_path path("tiff_reader_regions.tif");

	SECTION( "the whole of an image" )
	{
		const auto values = counting<float>(70, 0.5F);
		write_striped_file<float>(
			path.get(), "w", 7, 10, SAMPLEFORMAT_IEEEFP,
			COMPRESSION_ADOBE_DEFLATE, 4, {values}
		);

		const tiff_reader reader(path.get());

		REQUIRE( read_all<float>(reader, {10, 7}, numerical_type::float32) ==
			values );
	}

	SECTION( "the whole of a stack in one region" )
	{
		const auto values = write_stack(path.get(), COMPRESSION_LZW);

		const tiff_reader reader(path.get());

		REQUIRE( read_all<std::uint16_t>(
			reader, stack_extents, numerical_type::uint16) == values );
	}

	SECTION( "one page of a stack on its own" )
	{
		const auto values = write_stack(path.get(), COMPRESSION_LZW);

		const tiff_reader reader(path.get());

		const std::vector<std::size_t> region = {4, 5};
		auto destination =
			make_host_array<std::uint16_t>(region, numerical_type::uint16);
		image_transfer_plan regions(image_transfer_shape(region, 3, 2));
		regions.add(
			make_span(std::vector<std::size_t>{1, 0, 0}),
			make_span(std::vector<std::size_t>{0, 0})
		);

		reader.read(array_ref(destination), regions);

		REQUIRE( values_of<std::uint16_t>(destination, region) ==
			std::vector<std::uint16_t>(
				values.begin() + 20, values.begin() + 40) );
	}

	SECTION( "pages stated out of order arrive where they belong" )
	{
		const auto values = write_stack(path.get(), COMPRESSION_LZW);

		const tiff_reader reader(path.get());

		auto destination = make_host_array<std::uint16_t>(
			stack_extents, numerical_type::uint16);
		const std::vector<std::size_t> region = {4, 5};
		image_transfer_plan regions(image_transfer_shape(region, 3, 3));
		const std::size_t pages[3] = {2, 0, 1};
		for (std::size_t i = 0; i < 3; ++i)
		{
			regions.add(
				make_span(std::vector<std::size_t>{pages[i], 0, 0}),
				make_span(std::vector<std::size_t>{i, 0, 0})
			);
		}

		reader.read(array_ref(destination), regions);

		std::vector<std::uint16_t> expected;
		for (const auto page : pages)
		{
			expected.insert(
				expected.end(),
				values.begin() + static_cast<std::ptrdiff_t>(page * 20),
				values.begin() + static_cast<std::ptrdiff_t>(page * 20 + 20)
			);
		}

		REQUIRE( values_of<std::uint16_t>(destination, stack_extents) ==
			expected );
	}

	SECTION( "patches of an image, side by side in the array" )
	{
		// Rows 5 and 6 at columns 1 to 3, and rows 8 and 9 at columns 4
		// to 6, which lie in different strips.
		const auto values = counting<std::uint8_t>(70);
		write_striped_file<std::uint8_t>(
			path.get(), "w", 7, 10, SAMPLEFORMAT_UINT, COMPRESSION_LZW, 4,
			{values}
		);

		const tiff_reader reader(path.get());

		const std::vector<std::size_t> extents = {2, 2, 3};
		auto destination =
			make_host_array<std::uint8_t>(extents, numerical_type::uint8);
		image_transfer_plan regions(image_transfer_shape({2, 3}, 2, 3));
		regions.add(
			make_span(std::vector<std::size_t>{8, 4}),
			make_span(std::vector<std::size_t>{1, 0, 0})
		);
		regions.add(
			make_span(std::vector<std::size_t>{5, 1}),
			make_span(std::vector<std::size_t>{0, 0, 0})
		);

		reader.read(array_ref(destination), regions);

		REQUIRE( values_of<std::uint8_t>(destination, extents) ==
			std::vector<std::uint8_t>{
				36, 37, 38, 43, 44, 45,
				60, 61, 62, 67, 68, 69
			} );
	}

	SECTION( "a patch of a page cut into tiles" )
	{
		const auto values = counting<std::uint16_t>(2000);
		write_tiled_file<std::uint16_t>(
			path.get(), 50, 40, SAMPLEFORMAT_UINT, COMPRESSION_LZW, 16, 32,
			values
		);

		const tiff_reader reader(path.get());

		// Two rows from the thirty second, where two rows of tiles meet,
		// and three columns from the fifteenth, where two columns do.
		const std::vector<std::size_t> region = {2, 3};
		auto destination =
			make_host_array<std::uint16_t>(region, numerical_type::uint16);
		image_transfer_plan regions(image_transfer_shape(region, 2, 2));
		regions.add(
			make_span(std::vector<std::size_t>{31, 14}),
			make_span(std::vector<std::size_t>{0, 0})
		);

		reader.read(array_ref(destination), regions);

		REQUIRE( values_of<std::uint16_t>(destination, region) ==
			std::vector<std::uint16_t>{
				1564, 1565, 1566,
				1614, 1615, 1616
			} );
	}

	SECTION( "an empty plan reads nothing and succeeds" )
	{
		write_stack(path.get(), COMPRESSION_NONE);

		const tiff_reader reader(path.get());

		const std::vector<std::size_t> region = {4, 5};
		auto destination =
			make_host_array<std::uint16_t>(region, numerical_type::uint16);
		const image_transfer_plan regions(image_transfer_shape(region, 3, 2));

		REQUIRE_NOTHROW( reader.read(array_ref(destination), regions) );
	}
}

TEST_CASE( "a TIFF reader hands the values over as the array takes them",
	"[tiff_reader]" )
{
	const scoped_path path("tiff_reader_converted.tif");

	SECTION( "converted to the data type of the array" )
	{
		const std::vector<std::int16_t> stored = {
			-2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 300
		};
		write_striped_file<std::int16_t>(
			path.get(), "w", 4, 3, SAMPLEFORMAT_INT, COMPRESSION_LZW, 3,
			{stored}
		);

		const tiff_reader reader(path.get());

		REQUIRE( read_all<float>(reader, {3, 4}, numerical_type::float32) ==
			std::vector<float>(stored.cbegin(), stored.cend()) );
	}

	SECTION( "from a file of the other byte order" )
	{
		const auto values = counting<std::uint16_t>(12, 250);
		write_striped_file<std::uint16_t>(
			path.get(), "wb", 4, 3, SAMPLEFORMAT_UINT, COMPRESSION_NONE, 3,
			{values}
		);

		const tiff_reader reader(path.get());

		REQUIRE( read_all<std::uint16_t>(
			reader, {3, 4}, numerical_type::uint16) == values );
	}

	SECTION( "into an array whose axes are not stored in order" )
	{
		const auto values = counting<std::uint8_t>(12);
		write_striped_file<std::uint8_t>(
			path.get(), "w", 4, 3, SAMPLEFORMAT_UINT, COMPRESSION_NONE, 3,
			{values}
		);

		const tiff_reader reader(path.get());

		// Three rows of four columns held column after column, so that the
		// storage reads as the transpose of the image.
		const std::vector<std::size_t> extents = {3, 4};
		const std::vector<std::ptrdiff_t> strides = {1, 3};
		auto storage = make_host_array<std::uint8_t>(
			extents, numerical_type::uint8);
		array destination(
			storage.share_storage(),
			array_descriptor(
				strided_layout::make_custom_layout(
					make_span(extents), make_span(strides)),
				numerical_type::uint8
			)
		);

		reader.read(array_ref(destination), whole_of(extents));

		REQUIRE( values_of<std::uint8_t>(destination, extents) ==
			std::vector<std::uint8_t>{
				0, 4, 8, 1, 5, 9, 2, 6, 10, 3, 7, 11
			} );
	}
}

TEST_CASE( "a TIFF reader refuses what it can not read",
	"[tiff_reader]" )
{
	const scoped_path path("tiff_reader_refused.tif");

	SECTION( "a file that is not there" )
	{
		REQUIRE_THROWS_AS( tiff_reader(path.get()), image_file_error );
	}

	SECTION( "a file whose pages differ" )
	{
		write_striped_file<std::uint8_t>(
			path.get(), "w", 4, 2, SAMPLEFORMAT_UINT, COMPRESSION_NONE, 2,
			{counting<std::uint8_t>(8)}
		);
		append_striped_page<std::uint8_t>(
			path.get(), 5, 2, SAMPLEFORMAT_UINT, COMPRESSION_NONE, 2,
			counting<std::uint8_t>(10)
		);

		REQUIRE_THROWS_AS( tiff_reader(path.get()), image_format_error );
	}

	SECTION( "an uninitialized destination" )
	{
		write_stack(path.get(), COMPRESSION_NONE);

		const tiff_reader reader(path.get());

		REQUIRE_THROWS_AS(
			reader.read(array_ref(), whole_of(stack_extents)),
			std::invalid_argument
		);
	}

	SECTION( "regions that do not have the rank of the file" )
	{
		write_stack(path.get(), COMPRESSION_NONE);

		const tiff_reader reader(path.get());

		const std::vector<std::size_t> region = {4, 5};
		auto destination =
			make_host_array<std::uint16_t>(region, numerical_type::uint16);

		REQUIRE_THROWS_AS(
			reader.read(array_ref(destination), whole_of(region)),
			std::invalid_argument
		);
	}

	SECTION( "a page the file does not have, before anything is read" )
	{
		write_stack(path.get(), COMPRESSION_NONE);

		const tiff_reader reader(path.get());

		const std::vector<std::size_t> extents = {2, 4, 5};
		auto destination =
			make_host_array<std::uint16_t>(extents, numerical_type::uint16);
		image_transfer_plan regions(image_transfer_shape({4, 5}, 3, 3));
		regions.add(
			make_span(std::vector<std::size_t>{0, 0, 0}),
			make_span(std::vector<std::size_t>{0, 0, 0})
		);
		regions.add(
			make_span(std::vector<std::size_t>{3, 0, 0}),
			make_span(std::vector<std::size_t>{1, 0, 0})
		);

		REQUIRE_THROWS_AS(
			reader.read(array_ref(destination), regions),
			std::out_of_range
		);
		REQUIRE( values_of<std::uint16_t>(destination, extents) ==
			std::vector<std::uint16_t>(40, 0) );
	}

	SECTION( "a region reaching past the edge of a page" )
	{
		write_stack(path.get(), COMPRESSION_NONE);

		const tiff_reader reader(path.get());

		const std::vector<std::size_t> region = {4, 5};
		auto destination =
			make_host_array<std::uint16_t>(region, numerical_type::uint16);
		image_transfer_plan regions(image_transfer_shape(region, 3, 2));
		regions.add(
			make_span(std::vector<std::size_t>{0, 1, 0}),
			make_span(std::vector<std::size_t>{0, 0})
		);

		REQUIRE_THROWS_AS(
			reader.read(array_ref(destination), regions),
			std::out_of_range
		);
	}

	SECTION( "a region that does not fit in the array" )
	{
		write_stack(path.get(), COMPRESSION_NONE);

		const tiff_reader reader(path.get());

		const std::vector<std::size_t> extents = {4, 4};
		auto destination =
			make_host_array<std::uint16_t>(extents, numerical_type::uint16);
		image_transfer_plan regions(image_transfer_shape({4, 5}, 3, 2));
		regions.add(
			make_span(std::vector<std::size_t>{0, 0, 0}),
			make_span(std::vector<std::size_t>{0, 0})
		);

		REQUIRE_THROWS_AS(
			reader.read(array_ref(destination), regions),
			std::out_of_range
		);
	}
}

TEST_CASE( "a TIFF reader is read by several threads at once",
	"[tiff_reader]" )
{
	const scoped_path path("tiff_reader_concurrent.tif");
	const auto values = write_stack(path.get(), COMPRESSION_LZW);

	const tiff_reader reader(path.get());

	// Every thread reads its own page over and over into its own array, so
	// that the pages decoded keep displacing one another.
	const std::vector<std::size_t> region = {4, 5};
	std::vector<array> destinations;
	for (std::size_t page = 0; page < 3; ++page)
	{
		destinations.push_back(
			make_host_array<std::uint16_t>(region, numerical_type::uint16));
	}

	std::vector<std::thread> threads;
	for (std::size_t page = 0; page < 3; ++page)
	{
		threads.emplace_back(
			[&reader, &destinations, &region, page] ()
			{
				image_transfer_plan regions(
					image_transfer_shape(region, 3, 2));
				regions.add(
					make_span(std::vector<std::size_t>{page, 0, 0}),
					make_span(std::vector<std::size_t>{0, 0})
				);

				for (std::size_t repeat = 0; repeat < 50; ++repeat)
				{
					reader.read(array_ref(destinations[page]), regions);
				}
			}
		);
	}

	for (auto &thread : threads)
	{
		thread.join();
	}

	for (std::size_t page = 0; page < 3; ++page)
	{
		const auto first = static_cast<std::ptrdiff_t>(page * 20);

		REQUIRE( values_of<std::uint16_t>(destinations[page], region) ==
			std::vector<std::uint16_t>(
				values.begin() + first, values.begin() + first + 20) );
	}
}
