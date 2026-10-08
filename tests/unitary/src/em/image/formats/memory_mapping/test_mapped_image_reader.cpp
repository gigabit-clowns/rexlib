// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <em/image/formats/memory_mapping/mapped_image_reader.hpp>

#include <em/image/formats/memory_mapping/image_file_layout.hpp>

#include <core/hardware/host_memory/host_buffer.hpp>
#include <rexlib/core/exceptions/unsupported_operation_error.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/memory/byte_order.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/em/image/exceptions/image_file_error.hpp>
#include <rexlib/em/image/exceptions/image_format_error.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>

#include "../../fixtures/scoped_path.hpp"

#include <algorithm>
#include <array>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

// What every file below begins with, before its values. A multiple of the
// size of every element type the cases use.
const std::size_t preamble_size = 16;

std::size_t element_count(const std::vector<std::size_t> &extents)
{
	return std::accumulate(
		extents.cbegin(),
		extents.cend(),
		std::size_t(1),
		std::multiplies<std::size_t>()
	);
}

std::vector<float> counting(std::size_t count)
{
	std::vector<float> values(count);
	for (std::size_t i = 0; i < count; ++i)
	{
		values[i] = static_cast<float>(i);
	}

	return values;
}

// Write a preamble followed by the bytes of some values.
void write_file(
	const std::string &path,
	const void *values,
	std::size_t size
)
{
	std::vector<char> raw(preamble_size + size, 'p');
	std::memcpy(raw.data() + preamble_size, values, size);

	std::ofstream output(path.c_str(), std::ios::out | std::ios::binary);
	output.write(raw.data(), static_cast<std::streamsize>(raw.size()));
}

void write_file(const std::string &path, const std::vector<float> &values)
{
	write_file(path, values.data(), values.size() * sizeof(float));
}

std::vector<std::ptrdiff_t>
contiguous_strides(const std::vector<std::size_t> &extents)
{
	std::vector<std::ptrdiff_t> strides(extents.size());

	std::ptrdiff_t stride = 1;
	for (auto axis = extents.size(); axis > 0; --axis)
	{
		strides[axis - 1] = stride;
		stride *= static_cast<std::ptrdiff_t>(extents[axis - 1]);
	}

	return strides;
}

// A file of float32 values in the byte order of the host, laid out
// contiguously behind the preamble.
image_file_layout make_layout(
	const std::vector<std::size_t> &extents,
	std::size_t core_rank
)
{
	return image_file_layout(
		image_descriptor(
			make_span(extents),
			core_rank,
			numerical_type::float32
		),
		contiguous_strides(extents),
		preamble_size,
		get_system_byte_order()
	);
}

array make_host_array(const std::vector<std::size_t> &extents)
{
	auto storage = std::make_shared<host_buffer>(
		element_count(extents) * sizeof(float),
		alignof(float)
	);

	return array(
		std::move(storage),
		array_descriptor(
			strided_layout::make_contiguous_layout(make_span(extents)),
			numerical_type::float32
		)
	);
}

std::vector<float> values_of(
	const array &destination,
	const std::vector<std::size_t> &extents
)
{
	const auto *data = static_cast<const float*>(
		destination.get_storage()->get_host_ptr());

	return std::vector<float>(data, data + element_count(extents));
}

image_transfer_plan whole_of(const std::vector<std::size_t> &extents)
{
	image_transfer_plan regions(
		image_transfer_shape(extents, extents.size(), extents.size())
	);
	regions.add(
		make_span(std::vector<std::size_t>(extents.size(), 0)),
		make_span(std::vector<std::size_t>(extents.size(), 0))
	);

	return regions;
}

std::vector<float> read_all(
	const mapped_image_reader &reader,
	const std::vector<std::size_t> &extents
)
{
	auto destination = make_host_array(extents);
	reader.read(array_ref(destination), whole_of(extents));

	return values_of(destination, extents);
}

} // anonymous namespace

TEST_CASE(
	"a mapped_image_reader reports the descriptor of its layout",
	"[mapped_image_reader]"
)
{
	const scoped_path path("mapped_reader_descriptor.raw");
	const std::vector<std::size_t> extents = {2, 3, 4};
	write_file(path.get(), counting(24));

	const mapped_image_reader reader(
		path.get(),
		make_layout(extents, 2),
		image_metadata()
	);

	const image_descriptor expected(
		make_span(extents),
		2,
		numerical_type::float32
	);

	REQUIRE( reader.get_descriptor() == expected );
}

TEST_CASE(
	"a mapped_image_reader reads the values behind the preamble",
	"[mapped_image_reader]"
)
{
	const scoped_path path("mapped_reader_values.raw");

	SECTION( "the whole of a stack arrives in order" )
	{
		const std::vector<std::size_t> extents = {2, 3, 4};
		const auto values = counting(24);
		write_file(path.get(), values);

		const mapped_image_reader reader(
			path.get(),
			make_layout(extents, 2),
			image_metadata()
		);

		REQUIRE( read_all(reader, extents) == values );
	}

	SECTION( "one plane of a stack is read on its own" )
	{
		const std::vector<std::size_t> extents = {2, 3, 4};
		const auto values = counting(24);
		write_file(path.get(), values);

		const mapped_image_reader reader(
			path.get(),
			make_layout(extents, 2),
			image_metadata()
		);

		const std::vector<std::size_t> region = {3, 4};
		auto destination = make_host_array(region);

		image_transfer_plan regions(image_transfer_shape(region, 3, 2));
		regions.add(
			make_span(std::vector<std::size_t>{1, 0, 0}),
			make_span(std::vector<std::size_t>{0, 0})
		);

		reader.read(array_ref(destination), regions);

		REQUIRE( values_of(destination, region) ==
			std::vector<float>(values.begin() + 12, values.end()) );
	}

	SECTION( "regions stated out of order all arrive where they belong" )
	{
		// The reader orders the regions by their place in the file before it
		// walks them, so this is what guards that each one keeps the slot of
		// the array it was stated with.
		const std::vector<std::size_t> extents = {3, 3, 4};
		const auto values = counting(36);
		write_file(path.get(), values);

		const mapped_image_reader reader(
			path.get(),
			make_layout(extents, 2),
			image_metadata()
		);

		auto destination = make_host_array(extents);

		const std::vector<std::size_t> region = {3, 4};
		image_transfer_plan regions(image_transfer_shape(region, 3, 3));

		const std::array<std::size_t, 3> sections = {2, 0, 1};
		for (std::size_t i = 0; i < 3; ++i)
		{
			regions.add(
				make_span(std::vector<std::size_t>{sections[i], 0, 0}),
				make_span(std::vector<std::size_t>{i, 0, 0})
			);
		}

		reader.read(array_ref(destination), regions);

		std::vector<float> expected;
		for (const auto section : sections)
		{
			expected.insert(
				expected.end(),
				values.begin() + static_cast<std::ptrdiff_t>(section * 12),
				values.begin() + static_cast<std::ptrdiff_t>(section * 12 + 12)
			);
		}

		REQUIRE( values_of(destination, extents) == expected );
	}

	SECTION( "an empty plan reads nothing and succeeds" )
	{
		const std::vector<std::size_t> extents = {2, 3, 4};
		write_file(path.get(), counting(24));

		const mapped_image_reader reader(
			path.get(),
			make_layout(extents, 2),
			image_metadata()
		);

		const std::vector<std::size_t> region = {3, 4};
		auto destination = make_host_array(region);
		const image_transfer_plan regions(image_transfer_shape(region, 3, 2));

		REQUIRE_NOTHROW( reader.read(array_ref(destination), regions) );
	}
}

TEST_CASE(
	"a mapped_image_reader reads its values as the layout states them",
	"[mapped_image_reader]"
)
{
	const scoped_path path("mapped_reader_layout.raw");
	const std::vector<std::size_t> extents = {3, 4};

	SECTION( "along strides that are not descending" )
	{
		// The file stores its columns one after another, so the element of
		// row r and column c is the one at r + 3 c.
		const auto stored = counting(12);
		write_file(path.get(), stored);

		const mapped_image_reader reader(
			path.get(),
			image_file_layout(
				image_descriptor(
					make_span(extents),
					2,
					numerical_type::float32
				),
				std::vector<std::ptrdiff_t>{1, 3},
				preamble_size,
				get_system_byte_order()
			),
			image_metadata()
		);

		std::vector<float> expected;
		for (std::size_t row = 0; row < 3; ++row)
		{
			for (std::size_t column = 0; column < 4; ++column)
			{
				expected.push_back(stored[row + 3 * column]);
			}
		}

		REQUIRE( read_all(reader, extents) == expected );
	}

	SECTION( "in the other byte order" )
	{
		const auto values = counting(12);

		std::vector<float> swapped(values.size());
		std::memcpy(
			swapped.data(),
			values.data(),
			values.size() * sizeof(float)
		);
		auto *bytes = reinterpret_cast<char*>(swapped.data());
		for (std::size_t i = 0; i < swapped.size(); ++i)
		{
			std::reverse(
				bytes + i * sizeof(float),
				bytes + (i + 1) * sizeof(float)
			);
		}
		write_file(path.get(), swapped);

		const auto other =
			get_system_byte_order() == byte_order::little_endian
				? byte_order::big_endian
				: byte_order::little_endian;

		const mapped_image_reader reader(
			path.get(),
			image_file_layout(
				image_descriptor(
					make_span(extents),
					2,
					numerical_type::float32
				),
				contiguous_strides(extents),
				preamble_size,
				other
			),
			image_metadata()
		);

		REQUIRE( read_all(reader, extents) == values );
	}

	SECTION( "converted to the data type of the array" )
	{
		const std::vector<std::int16_t> stored = {
			-2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 300
		};
		write_file(
			path.get(),
			stored.data(),
			stored.size() * sizeof(std::int16_t)
		);

		const mapped_image_reader reader(
			path.get(),
			image_file_layout(
				image_descriptor(
					make_span(extents),
					2,
					numerical_type::int16
				),
				contiguous_strides(extents),
				preamble_size,
				get_system_byte_order()
			),
			image_metadata()
		);

		REQUIRE( read_all(reader, extents) ==
			std::vector<float>(stored.cbegin(), stored.cend()) );
	}
}

TEST_CASE(
	"a mapped_image_reader refuses what it can not read",
	"[mapped_image_reader]"
)
{
	const scoped_path path("mapped_reader_refused.raw");
	const std::vector<std::size_t> extents = {3, 4};

	SECTION( "an uninitialized destination" )
	{
		write_file(path.get(), counting(12));

		const mapped_image_reader reader(
			path.get(),
			make_layout(extents, 2),
			image_metadata()
		);

		REQUIRE_THROWS_AS(
			reader.read(array_ref(), whole_of(extents)),
			std::invalid_argument
		);
	}

	SECTION( "a region the file does not contain" )
	{
		write_file(path.get(), counting(12));

		const mapped_image_reader reader(
			path.get(),
			make_layout(extents, 2),
			image_metadata()
		);

		auto destination = make_host_array(extents);
		image_transfer_plan regions(image_transfer_shape(extents, 2, 2));
		regions.add(
			make_span(std::vector<std::size_t>{1, 0}),
			make_span(std::vector<std::size_t>{0, 0})
		);

		REQUIRE_THROWS_AS(
			reader.read(array_ref(destination), regions),
			std::out_of_range
		);
	}

	SECTION( "a data type the array's can not be produced from" )
	{
		const std::vector<std::complex<float>> stored(12);
		write_file(
			path.get(),
			stored.data(),
			stored.size() * sizeof(std::complex<float>)
		);

		const mapped_image_reader reader(
			path.get(),
			image_file_layout(
				image_descriptor(
					make_span(extents),
					2,
					numerical_type::complex_float32
				),
				contiguous_strides(extents),
				preamble_size,
				get_system_byte_order()
			),
			image_metadata()
		);

		auto destination = make_host_array(extents);

		REQUIRE_THROWS_AS(
			reader.read(array_ref(destination), whole_of(extents)),
			unsupported_operation_error
		);
	}
}

TEST_CASE(
	"a mapped_image_reader refuses a file that can not hold its layout, "
	"naming it",
	"[mapped_image_reader]"
)
{
	const scoped_path path("mapped_reader_bad_file.raw");
	const std::vector<std::size_t> extents = {2, 3, 4};
	const auto names_the_file = Catch::Matchers::MessageMatches(
		Catch::Matchers::StartsWith(path.get() + ": ")
	);

	SECTION( "one shorter than the values it is said to hold" )
	{
		write_file(path.get(), counting(23));

		REQUIRE_THROWS_MATCHES(
			mapped_image_reader(
				path.get(),
				make_layout(extents, 2),
				image_metadata()
			),
			image_format_error,
			names_the_file
		);
	}

	SECTION( "one that is not there" )
	{
		REQUIRE_THROWS_MATCHES(
			mapped_image_reader(
				path.get(),
				make_layout(extents, 2),
				image_metadata()
			),
			image_file_error,
			names_the_file
		);
	}
}
