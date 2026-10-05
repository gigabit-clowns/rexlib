// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <em/image/formats/tiff/tiff_writer.hpp>

#include <em/image/formats/tiff/tiff_reader.hpp>

#include <rexlib/core/exceptions/unsupported_operation_error.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/em/image/exceptions/image_file_error.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>

#include "../../fixtures/scoped_path.hpp"
#include "fixtures/host_array.hpp"
#include "fixtures/tiff_test_file.hpp"

#include <complex>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;
using namespace rexlib::em::tiff;
using namespace rexlib::test;

namespace
{

// A stack of three pages of four rows and five columns.
const std::vector<std::size_t> stack_extents = {3, 4, 5};
const std::vector<std::size_t> page_extents = {4, 5};

image_descriptor describe(
	const std::vector<std::size_t> &extents,
	numerical_type data_type
)
{
	return image_descriptor(make_span(extents), 2, data_type);
}

template <typename T>
std::vector<T> read_back(
	const std::string &path,
	const std::vector<std::size_t> &extents,
	numerical_type data_type
)
{
	const tiff_reader reader(path);
	auto destination = make_host_array<T>(extents, data_type);
	reader.read(array_ref(destination), whole_of(extents));

	return values_of<T>(destination, extents);
}

std::vector<std::size_t> extents_of(const std::string &path)
{
	const tiff_reader reader(path);
	const auto extents = reader.get_descriptor().get_extents();

	return std::vector<std::size_t>(extents.begin(), extents.end());
}

// What a file states of itself, read through libtiff rather than through
// this project's reader.
std::uint16_t compression_of(const std::string &path)
{
	auto *file = TIFFOpen(path.c_str(), "r");
	std::uint16_t compression = 0;
	TIFFGetField(file, TIFFTAG_COMPRESSION, &compression);
	TIFFClose(file);

	return compression;
}

// The plan of pages of a stack taken from the planes of an array of rank
// three, one region per pair of a page and the plane it comes from.
image_transfer_plan pages_from(
	const std::vector<std::pair<std::size_t, std::size_t>> &pairs
)
{
	image_transfer_plan regions(image_transfer_shape(page_extents, 3, 3));
	for (const auto &pair : pairs)
	{
		regions.add(
			make_span(std::vector<std::size_t>{pair.first, 0, 0}),
			make_span(std::vector<std::size_t>{pair.second, 0, 0})
		);
	}

	return regions;
}

} // anonymous namespace

TEST_CASE( "a TIFF writer reports the descriptor it was created over",
	"[tiff_writer]" )
{
	const scoped_path path("tiff_writer_descriptor.tif");
	const auto descriptor = describe(stack_extents, numerical_type::int16);

	const tiff_writer writer(path.get(), descriptor);

	REQUIRE( writer.get_descriptor() == descriptor );
}

TEST_CASE( "a TIFF writer writes a file that reads back as it was written",
	"[tiff_writer]" )
{
	const scoped_path path("tiff_writer_written.tif");

	SECTION( "an image" )
	{
		const auto values = counting<std::uint8_t>(20);
		const auto source = make_host_array<std::uint8_t>(
			page_extents, numerical_type::uint8, values);

		{
			tiff_writer writer(
				path.get(), describe(page_extents, numerical_type::uint8));
			writer.write(const_array_ref(source), whole_of(page_extents));
		}

		REQUIRE( extents_of(path.get()) == page_extents );
		REQUIRE( read_back<std::uint8_t>(
			path.get(), page_extents, numerical_type::uint8) == values );
	}

	SECTION( "a stack in one region" )
	{
		const auto values = counting<std::int16_t>(60, -30);
		const auto source = make_host_array<std::int16_t>(
			stack_extents, numerical_type::int16, values);

		{
			tiff_writer writer(
				path.get(), describe(stack_extents, numerical_type::int16));
			writer.write(const_array_ref(source), whole_of(stack_extents));
		}

		REQUIRE( extents_of(path.get()) == stack_extents );
		REQUIRE( read_back<std::int16_t>(
			path.get(), stack_extents, numerical_type::int16) == values );
	}

	SECTION( "a stack a page at a time" )
	{
		const auto values = counting<std::uint16_t>(60);
		const auto source = make_host_array<std::uint16_t>(
			stack_extents, numerical_type::uint16, values);

		{
			tiff_writer writer(
				path.get(), describe(stack_extents, numerical_type::uint16));
			for (std::size_t page = 0; page < 3; ++page)
			{
				writer.write(
					const_array_ref(source), pages_from({{page, page}}));
			}
		}

		REQUIRE( read_back<std::uint16_t>(
			path.get(), stack_extents, numerical_type::uint16) == values );
	}

	SECTION( "a stack across calls of several pages" )
	{
		const auto values = counting<std::uint16_t>(60);
		const auto source = make_host_array<std::uint16_t>(
			stack_extents, numerical_type::uint16, values);

		{
			tiff_writer writer(
				path.get(), describe(stack_extents, numerical_type::uint16));

			image_transfer_plan first(
				image_transfer_shape({2, 4, 5}, 3, 3));
			first.add(
				make_span(std::vector<std::size_t>{0, 0, 0}),
				make_span(std::vector<std::size_t>{0, 0, 0})
			);
			writer.write(const_array_ref(source), first);
			writer.write(const_array_ref(source), pages_from({{2, 2}}));
		}

		REQUIRE( read_back<std::uint16_t>(
			path.get(), stack_extents, numerical_type::uint16) == values );
	}

	SECTION( "pages stated out of order within one call" )
	{
		// Page 1 comes from the first plane of the array, page 0 from its
		// last one and page 2 from the one in between.
		const auto values = counting<std::uint16_t>(60);
		const auto source = make_host_array<std::uint16_t>(
			stack_extents, numerical_type::uint16, values);

		{
			tiff_writer writer(
				path.get(), describe(stack_extents, numerical_type::uint16));
			writer.write(
				const_array_ref(source),
				pages_from({{1, 0}, {0, 2}, {2, 1}})
			);
		}

		std::vector<std::uint16_t> expected(values.begin() + 40, values.end());
		expected.insert(
			expected.end(), values.begin(), values.begin() + 20);
		expected.insert(
			expected.end(), values.begin() + 20, values.begin() + 40);

		REQUIRE( read_back<std::uint16_t>(
			path.get(), stack_extents, numerical_type::uint16) == expected );
	}

	SECTION( "an image a strip can not hold whole" )
	{
		// A row of this image is 128 KiB, so a strip holds two of them and
		// the image takes several strips, the last one shorter.
		const std::vector<std::size_t> extents = {5, 65536};
		const auto values = counting<std::uint16_t>(5 * 65536);
		const auto source = make_host_array<std::uint16_t>(
			extents, numerical_type::uint16, values);

		{
			tiff_writer writer(
				path.get(), describe(extents, numerical_type::uint16));
			writer.write(const_array_ref(source), whole_of(extents));
		}

		REQUIRE( read_back<std::uint16_t>(
			path.get(), extents, numerical_type::uint16) == values );
	}
}

TEST_CASE( "a TIFF writer takes the values as the array holds them",
	"[tiff_writer]" )
{
	const scoped_path path("tiff_writer_converted.tif");

	SECTION( "converted to the data type of the file" )
	{
		const std::vector<float> held = {
			-2.0F, -1.0F, 0.0F, 1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F,
			8.0F, 9.0F, 10.0F, 11.0F, 12.0F, 13.0F, 14.0F, 15.0F, 16.0F,
			300.0F
		};
		const auto source = make_host_array<float>(
			page_extents, numerical_type::float32, held);

		{
			tiff_writer writer(
				path.get(), describe(page_extents, numerical_type::int16));
			writer.write(const_array_ref(source), whole_of(page_extents));
		}

		REQUIRE( read_back<std::int16_t>(
			path.get(), page_extents, numerical_type::int16) ==
			std::vector<std::int16_t>(held.cbegin(), held.cend()) );
	}

	SECTION( "from an array whose axes are not stored in order" )
	{
		// Four rows of five columns held column after column.
		const std::vector<std::ptrdiff_t> strides = {1, 4};
		const auto held = counting<std::uint8_t>(20);
		auto storage = make_host_array<std::uint8_t>(
			page_extents, numerical_type::uint8, held);
		const array source(
			storage.share_storage(),
			array_descriptor(
				strided_layout::make_custom_layout(
					make_span(page_extents), make_span(strides)),
				numerical_type::uint8
			)
		);

		{
			tiff_writer writer(
				path.get(), describe(page_extents, numerical_type::uint8));
			writer.write(const_array_ref(source), whole_of(page_extents));
		}

		REQUIRE( read_back<std::uint8_t>(
			path.get(), page_extents, numerical_type::uint8) ==
			std::vector<std::uint8_t>{
				0, 4, 8, 12, 16,
				1, 5, 9, 13, 17,
				2, 6, 10, 14, 18,
				3, 7, 11, 15, 19
			} );
	}
}

TEST_CASE( "a TIFF writer encodes a file by what it holds",
	"[tiff_writer]" )
{
	const scoped_path path("tiff_writer_encoded.tif");

	SECTION( "integer samples are compressed" )
	{
		const auto source = make_host_array<std::uint8_t>(
			page_extents, numerical_type::uint8);

		{
			tiff_writer writer(
				path.get(), describe(page_extents, numerical_type::uint8));
			writer.write(const_array_ref(source), whole_of(page_extents));
		}

		REQUIRE( compression_of(path.get()) == COMPRESSION_LZW );
	}

	SECTION( "wide and complex samples are written as any other" )
	{
		const auto integers = counting<std::uint64_t>(
			20, 18000000000000000000ULL);
		const auto reals = counting<double>(20, 0.125);
		std::vector<std::complex<double>> complexes;
		for (std::size_t i = 0; i < 20; ++i)
		{
			complexes.emplace_back(
				static_cast<double>(i), -0.5 * static_cast<double>(i));
		}

		{
			const auto source = make_host_array<std::uint64_t>(
				page_extents, numerical_type::uint64, integers);
			tiff_writer writer(
				path.get(), describe(page_extents, numerical_type::uint64));
			writer.write(const_array_ref(source), whole_of(page_extents));
		}

		REQUIRE( compression_of(path.get()) == COMPRESSION_LZW );
		REQUIRE( read_back<std::uint64_t>(
			path.get(), page_extents, numerical_type::uint64) == integers );

		{
			const auto source = make_host_array<double>(
				page_extents, numerical_type::float64, reals);
			tiff_writer writer(
				path.get(), describe(page_extents, numerical_type::float64));
			writer.write(const_array_ref(source), whole_of(page_extents));
		}

		REQUIRE( compression_of(path.get()) == COMPRESSION_NONE );
		REQUIRE( read_back<double>(
			path.get(), page_extents, numerical_type::float64) == reals );

		{
			const auto source = make_host_array<std::complex<double>>(
				page_extents, numerical_type::complex_float64, complexes);
			tiff_writer writer(
				path.get(),
				describe(page_extents, numerical_type::complex_float64)
			);
			writer.write(const_array_ref(source), whole_of(page_extents));
		}

		REQUIRE( compression_of(path.get()) == COMPRESSION_NONE );
		REQUIRE( read_back<std::complex<double>>(
			path.get(), page_extents, numerical_type::complex_float64) ==
			complexes );
	}

	SECTION( "floating point samples are not" )
	{
		const auto values = counting<float>(20, 0.25F);
		const auto source = make_host_array<float>(
			page_extents, numerical_type::float32, values);

		{
			tiff_writer writer(
				path.get(), describe(page_extents, numerical_type::float32));
			writer.write(const_array_ref(source), whole_of(page_extents));
		}

		REQUIRE( compression_of(path.get()) == COMPRESSION_NONE );
		REQUIRE( read_back<float>(
			path.get(), page_extents, numerical_type::float32) == values );
	}

	SECTION( "a file that fits is created as a classic one" )
	{
		const auto source = make_host_array<std::uint8_t>(
			page_extents, numerical_type::uint8);

		{
			tiff_writer writer(
				path.get(), describe(page_extents, numerical_type::uint8));
			writer.write(const_array_ref(source), whole_of(page_extents));
		}

		const auto raw = read_file(path.get());

		REQUIRE( raw[2] + raw[3] == 42 );
	}

	SECTION( "a file that might not fit is created as a BigTIFF one" )
	{
		// A stack declared at three gigabytes, of which a single page of
		// 64 KiB is written.
		const std::vector<std::size_t> extents = {49152, 256, 256};
		const std::vector<std::size_t> plane = {1, 256, 256};
		const auto source = make_host_array<std::uint8_t>(
			plane, numerical_type::uint8);

		{
			tiff_writer writer(
				path.get(), describe(extents, numerical_type::uint8));
			image_transfer_plan regions(
				image_transfer_shape({256, 256}, 3, 3));
			regions.add(
				make_span(std::vector<std::size_t>{0, 0, 0}),
				make_span(std::vector<std::size_t>{0, 0, 0})
			);
			writer.write(const_array_ref(source), regions);
		}

		const auto raw = read_file(path.get());

		REQUIRE( raw[2] + raw[3] == 43 );
	}
}

TEST_CASE( "a TIFF writer refuses to write part of a page",
	"[tiff_writer]" )
{
	const scoped_path path("tiff_writer_partial.tif");
	const auto values = counting<std::uint16_t>(60);
	const auto source = make_host_array<std::uint16_t>(
		stack_extents, numerical_type::uint16, values);

	tiff_writer writer(
		path.get(), describe(stack_extents, numerical_type::uint16));

	SECTION( "some rows of a page" )
	{
		image_transfer_plan regions(image_transfer_shape({2, 5}, 3, 3));
		regions.add(
			make_span(std::vector<std::size_t>{0, 0, 0}),
			make_span(std::vector<std::size_t>{0, 0, 0})
		);

		REQUIRE_THROWS_MATCHES(
			writer.write(const_array_ref(source), regions),
			unsupported_operation_error,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::ContainsSubstring("whole pages")
			)
		);
	}

	SECTION( "some columns of a page" )
	{
		image_transfer_plan regions(image_transfer_shape({4, 3}, 3, 3));
		regions.add(
			make_span(std::vector<std::size_t>{0, 0, 1}),
			make_span(std::vector<std::size_t>{0, 0, 0})
		);

		REQUIRE_THROWS_AS(
			writer.write(const_array_ref(source), regions),
			unsupported_operation_error
		);
	}

	SECTION( "a single row" )
	{
		image_transfer_plan regions(image_transfer_shape({5}, 3, 3));
		regions.add(
			make_span(std::vector<std::size_t>{0, 0, 0}),
			make_span(std::vector<std::size_t>{0, 0, 0})
		);

		REQUIRE_THROWS_AS(
			writer.write(const_array_ref(source), regions),
			unsupported_operation_error
		);
	}

	SECTION( "the file is still written from its first page afterwards" )
	{
		image_transfer_plan regions(image_transfer_shape({2, 5}, 3, 3));
		regions.add(
			make_span(std::vector<std::size_t>{0, 0, 0}),
			make_span(std::vector<std::size_t>{0, 0, 0})
		);

		REQUIRE_THROWS_AS(
			writer.write(const_array_ref(source), regions),
			unsupported_operation_error
		);
		REQUIRE_NOTHROW(
			writer.write(const_array_ref(source), whole_of(stack_extents)) );
	}
}

TEST_CASE( "a TIFF writer refuses pages out of order or written twice",
	"[tiff_writer]" )
{
	const scoped_path path("tiff_writer_ordered.tif");
	const auto values = counting<std::uint16_t>(60);
	const auto source = make_host_array<std::uint16_t>(
		stack_extents, numerical_type::uint16, values);

	SECTION( "a call that does not begin at the first page" )
	{
		tiff_writer writer(
			path.get(), describe(stack_extents, numerical_type::uint16));

		REQUIRE_THROWS_MATCHES(
			writer.write(const_array_ref(source), pages_from({{1, 1}})),
			unsupported_operation_error,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::ContainsSubstring("beginning at page 0")
			)
		);
	}

	SECTION( "a call that skips the next page" )
	{
		tiff_writer writer(
			path.get(), describe(stack_extents, numerical_type::uint16));
		writer.write(const_array_ref(source), pages_from({{0, 0}}));

		REQUIRE_THROWS_MATCHES(
			writer.write(const_array_ref(source), pages_from({{2, 2}})),
			unsupported_operation_error,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::ContainsSubstring("beginning at page 1")
			)
		);
	}

	SECTION( "a call that leaves a gap between its pages" )
	{
		tiff_writer writer(
			path.get(), describe(stack_extents, numerical_type::uint16));

		REQUIRE_THROWS_AS(
			writer.write(
				const_array_ref(source), pages_from({{0, 0}, {2, 2}})),
			unsupported_operation_error
		);
	}

	SECTION( "a page that has already been written" )
	{
		tiff_writer writer(
			path.get(), describe(stack_extents, numerical_type::uint16));
		writer.write(const_array_ref(source), pages_from({{0, 0}}));

		REQUIRE_THROWS_AS(
			writer.write(const_array_ref(source), pages_from({{0, 1}})),
			unsupported_operation_error
		);
	}

	SECTION( "a page stated twice in one call" )
	{
		tiff_writer writer(
			path.get(), describe(stack_extents, numerical_type::uint16));

		REQUIRE_THROWS_AS(
			writer.write(
				const_array_ref(source), pages_from({{0, 0}, {0, 1}})),
			unsupported_operation_error
		);
	}

	SECTION( "a call that is refused writes none of its pages" )
	{
		{
			tiff_writer writer(
				path.get(), describe(stack_extents, numerical_type::uint16));
			writer.write(const_array_ref(source), pages_from({{0, 0}}));

			// Page 1 is in turn, and page 0 after it is not.
			REQUIRE_THROWS_AS(
				writer.write(
					const_array_ref(source), pages_from({{1, 1}, {0, 2}})),
				unsupported_operation_error
			);

			writer.write(
				const_array_ref(source), pages_from({{1, 1}, {2, 2}}));
		}

		REQUIRE( read_back<std::uint16_t>(
			path.get(), stack_extents, numerical_type::uint16) == values );
	}
}

TEST_CASE( "a TIFF writer refuses what the file or the array can not take",
	"[tiff_writer]" )
{
	const scoped_path path("tiff_writer_refused.tif");
	const auto source = make_host_array<std::uint16_t>(
		stack_extents, numerical_type::uint16);

	tiff_writer writer(
		path.get(), describe(stack_extents, numerical_type::uint16));

	SECTION( "an uninitialized source" )
	{
		REQUIRE_THROWS_AS(
			writer.write(const_array_ref(), whole_of(stack_extents)),
			std::invalid_argument
		);
	}

	SECTION( "regions that do not have the rank of the file" )
	{
		REQUIRE_THROWS_AS(
			writer.write(const_array_ref(source), whole_of(page_extents)),
			std::invalid_argument
		);
	}

	SECTION( "a page the file does not have" )
	{
		REQUIRE_THROWS_AS(
			writer.write(const_array_ref(source), pages_from({{3, 0}})),
			std::out_of_range
		);
	}

	SECTION( "a whole page placed off the first row" )
	{
		image_transfer_plan regions(image_transfer_shape(page_extents, 3, 3));
		regions.add(
			make_span(std::vector<std::size_t>{0, 1, 0}),
			make_span(std::vector<std::size_t>{0, 0, 0})
		);

		REQUIRE_THROWS_AS(
			writer.write(const_array_ref(source), regions),
			std::out_of_range
		);
	}

	SECTION( "a page the array does not hold" )
	{
		REQUIRE_THROWS_AS(
			writer.write(const_array_ref(source), pages_from({{0, 3}})),
			std::out_of_range
		);
	}

	SECTION( "an empty plan writes nothing and succeeds" )
	{
		const image_transfer_plan regions(
			image_transfer_shape(page_extents, 3, 3));

		REQUIRE_NOTHROW( writer.write(const_array_ref(source), regions) );
		REQUIRE_NOTHROW(
			writer.write(const_array_ref(source), pages_from({{0, 0}})) );
	}
}

TEST_CASE( "a TIFF writer is not created over what a file can not hold",
	"[tiff_writer]" )
{
	const scoped_path path("tiff_writer_uncreated.tif");

	SECTION( "a single row" )
	{
		const std::vector<std::size_t> extents = {5};

		REQUIRE_THROWS_AS(
			tiff_writer(
				path.get(),
				image_descriptor(
					make_span(extents), 1, numerical_type::uint8)
			),
			std::invalid_argument
		);
	}

	SECTION( "a volume" )
	{
		REQUIRE_THROWS_AS(
			tiff_writer(
				path.get(),
				image_descriptor(
					make_span(stack_extents), 3, numerical_type::uint8)
			),
			std::invalid_argument
		);
	}

	SECTION( "a stack of stacks" )
	{
		const std::vector<std::size_t> extents = {2, 3, 4, 5};

		REQUIRE_THROWS_AS(
			tiff_writer(
				path.get(), describe(extents, numerical_type::uint8)),
			std::invalid_argument
		);
	}

	SECTION( "an extent of zero" )
	{
		const std::vector<std::size_t> extents = {3, 0, 5};

		REQUIRE_THROWS_AS(
			tiff_writer(
				path.get(), describe(extents, numerical_type::uint8)),
			std::invalid_argument
		);
	}

	SECTION( "a data type with no sample" )
	{
		REQUIRE_THROWS_AS(
			tiff_writer(
				path.get(), describe(page_extents, numerical_type::boolean)),
			unsupported_operation_error
		);
	}

	SECTION( "a path where there is no directory" )
	{
		const auto nowhere =
			get_scratch_path("tiff_writer_no_such_directory") + "/a.tif";

		REQUIRE_THROWS_AS(
			tiff_writer(
				nowhere, describe(page_extents, numerical_type::uint8)),
			image_file_error
		);
	}
}

TEST_CASE( "a TIFF writer holds what was written however it is left",
	"[tiff_writer]" )
{
	const scoped_path path("tiff_writer_left.tif");
	const auto values = counting<std::uint16_t>(60);
	const auto source = make_host_array<std::uint16_t>(
		stack_extents, numerical_type::uint16, values);

	SECTION( "flushing between pages changes nothing" )
	{
		{
			tiff_writer writer(
				path.get(), describe(stack_extents, numerical_type::uint16));
			writer.flush();
			writer.write(const_array_ref(source), pages_from({{0, 0}}));
			writer.flush();
			writer.write(
				const_array_ref(source), pages_from({{1, 1}, {2, 2}}));
			writer.flush();
		}

		REQUIRE( read_back<std::uint16_t>(
			path.get(), stack_extents, numerical_type::uint16) == values );
	}

	SECTION( "a file left short of pages holds the ones written" )
	{
		{
			tiff_writer writer(
				path.get(), describe(stack_extents, numerical_type::uint16));
			writer.write(
				const_array_ref(source), pages_from({{0, 0}, {1, 1}}));
		}

		const std::vector<std::size_t> written = {2, 4, 5};

		REQUIRE( extents_of(path.get()) == written );
		REQUIRE( read_back<std::uint16_t>(
			path.get(), written, numerical_type::uint16) ==
			std::vector<std::uint16_t>(
				values.begin(), values.begin() + 40) );
	}

	SECTION( "creating a writer replaces the file that was there" )
	{
		write_striped_file<std::uint8_t>(
			path.get(), "w", 7, 10, SAMPLEFORMAT_UINT, COMPRESSION_NONE, 4,
			{counting<std::uint8_t>(70), counting<std::uint8_t>(70)}
		);

		{
			tiff_writer writer(
				path.get(), describe(stack_extents, numerical_type::uint16));
			writer.write(const_array_ref(source), whole_of(stack_extents));
		}

		REQUIRE( extents_of(path.get()) == stack_extents );
	}
}

TEST_CASE( "a TIFF writer written by several threads keeps its pages in order",
	"[tiff_writer]" )
{
	const scoped_path path("tiff_writer_concurrent.tif");
	const std::vector<std::size_t> extents = {8, 4, 5};
	const auto values = counting<std::uint16_t>(160);
	const auto source = make_host_array<std::uint16_t>(
		extents, numerical_type::uint16, values);

	{
		tiff_writer writer(
			path.get(), describe(extents, numerical_type::uint16));

		// Every thread has one page to write and keeps offering it until
		// its turn comes, so the calls arrive in whatever order they do.
		std::vector<std::thread> threads;
		for (std::size_t page = 0; page < 8; ++page)
		{
			threads.emplace_back(
				[&writer, &source, page] ()
				{
					const auto regions = pages_from({{page, page}});
					for (;;)
					{
						try
						{
							writer.write(const_array_ref(source), regions);
							return;
						}
						catch (const unsupported_operation_error&)
						{
							std::this_thread::yield();
						}
					}
				}
			);
		}

		for (auto &thread : threads)
		{
			thread.join();
		}
	}

	REQUIRE( read_back<std::uint16_t>(
		path.get(), extents, numerical_type::uint16) == values );
}
