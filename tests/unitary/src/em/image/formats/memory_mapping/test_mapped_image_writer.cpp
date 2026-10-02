// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/formats/memory_mapping/mapped_image_writer.hpp>

#include <em/image/formats/memory_mapping/image_file_layout.hpp>

#include <core/hardware/host_memory/host_buffer.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/memory/byte.hpp>
#include <rexlib/core/memory/byte_order.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>

#include "../../fixtures/scoped_path.hpp"

#include <boost/filesystem/operations.hpp>

#include <algorithm>
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
const std::vector<rexlib::byte> preamble(16, static_cast<rexlib::byte>('p'));

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
		values[i] = static_cast<float>(i) + 0.5F;
	}

	return values;
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

image_descriptor make_descriptor(
	const std::vector<std::size_t> &extents,
	std::size_t core_rank,
	numerical_type data_type = numerical_type::float32
)
{
	return image_descriptor(make_span(extents), core_rank, data_type);
}

// A file of float32 values in the byte order of the host, laid out
// contiguously behind the preamble.
image_file_layout make_layout(
	const std::vector<std::size_t> &extents,
	std::size_t core_rank
)
{
	return image_file_layout(
		make_descriptor(extents, core_rank),
		contiguous_strides(extents),
		preamble.size(),
		get_system_byte_order()
	);
}

array make_host_array(
	const std::vector<std::size_t> &extents,
	const std::vector<float> &values
)
{
	auto storage = std::make_shared<host_buffer>(
		element_count(extents) * sizeof(float),
		alignof(float)
	);
	std::memcpy(
		storage->get_host_ptr(),
		values.data(),
		values.size() * sizeof(float)
	);

	return array(
		std::move(storage),
		array_descriptor(
			strided_layout::make_contiguous_layout(make_span(extents)),
			numerical_type::float32
		)
	);
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

// The bytes of a file, from its first one.
std::vector<char> bytes_of(const std::string &path)
{
	std::ifstream input(path.c_str(), std::ios::in | std::ios::binary);

	return std::vector<char>(
		std::istreambuf_iterator<char>(input),
		std::istreambuf_iterator<char>()
	);
}

// The values a file holds behind its preamble, as the host reads them.
std::vector<float> floats_of(const std::string &path, std::size_t count)
{
	const auto raw = bytes_of(path);

	std::vector<float> values(count);
	std::memcpy(
		values.data(),
		raw.data() + preamble.size(),
		count * sizeof(float)
	);

	return values;
}

} // anonymous namespace

TEST_CASE(
	"a mapped_image_writer reports the descriptor of its layout",
	"[mapped_image_writer]"
)
{
	const scoped_path path("mapped_writer_descriptor.raw");
	const std::vector<std::size_t> extents = {2, 3, 4};

	const mapped_image_writer writer(
		path.get(),
		make_layout(extents, 2),
		make_span(preamble.data(), preamble.size())
	);

	REQUIRE( writer.get_descriptor() == make_descriptor(extents, 2) );
}

TEST_CASE(
	"a mapped_image_writer lays its file out in full when it is constructed",
	"[mapped_image_writer]"
)
{
	const scoped_path path("mapped_writer_laid_out.raw");
	const std::vector<std::size_t> extents = {2, 3, 4};

	SECTION( "the file has room for the preamble and every value" )
	{
		{
			const mapped_image_writer writer(
				path.get(),
				make_layout(extents, 2),
				make_span(preamble.data(), preamble.size())
			);
		}

		REQUIRE( boost::filesystem::file_size(path.get()) ==
			preamble.size() + 24 * sizeof(float) );
	}

	SECTION( "the file begins with the preamble" )
	{
		{
			const mapped_image_writer writer(
				path.get(),
				make_layout(extents, 2),
				make_span(preamble.data(), preamble.size())
			);
		}

		const auto raw = bytes_of(path.get());

		REQUIRE( std::vector<char>(raw.begin(), raw.begin() + 16) ==
			std::vector<char>(16, 'p') );
	}

	SECTION( "a preamble shorter than the gap before the values fits" )
	{
		REQUIRE_NOTHROW(
			mapped_image_writer(
				path.get(),
				make_layout(extents, 2),
				make_span(preamble.data(), 4)
			)
		);
	}

	SECTION( "a file already there is replaced" )
	{
		{
			std::ofstream output(
				path.get().c_str(),
				std::ios::out | std::ios::binary
			);
			output << std::string(8192, 'x');
		}

		{
			const mapped_image_writer writer(
				path.get(),
				make_layout(extents, 2),
				make_span(preamble.data(), preamble.size())
			);
		}

		REQUIRE( boost::filesystem::file_size(path.get()) ==
			preamble.size() + 24 * sizeof(float) );
	}
}

TEST_CASE(
	"a mapped_image_writer refuses a preamble that reaches past where the "
	"values begin, creating nothing",
	"[mapped_image_writer]"
)
{
	const scoped_path path("mapped_writer_long_preamble.raw");
	const std::vector<std::size_t> extents = {3, 4};
	const std::vector<rexlib::byte> too_long(20);

	REQUIRE_THROWS_AS(
		mapped_image_writer(
			path.get(),
			make_layout(extents, 2),
			make_span(too_long.data(), too_long.size())
		),
		std::invalid_argument
	);
	REQUIRE_FALSE( boost::filesystem::exists(path.get()) );
}

TEST_CASE(
	"a mapped_image_writer writes its values behind the preamble",
	"[mapped_image_writer]"
)
{
	const scoped_path path("mapped_writer_values.raw");

	SECTION( "the whole of an array arrives in order" )
	{
		const std::vector<std::size_t> extents = {2, 3, 4};
		const auto values = counting(24);

		{
			mapped_image_writer writer(
				path.get(),
				make_layout(extents, 2),
				make_span(preamble.data(), preamble.size())
			);
			const auto source = make_host_array(extents, values);
			writer.write(const_array_ref(source), whole_of(extents));
			writer.flush();
		}

		REQUIRE( floats_of(path.get(), 24) == values );
	}

	SECTION( "regions written one at a time fill the file" )
	{
		const std::vector<std::size_t> extents = {3, 2, 2};
		const std::vector<std::size_t> plane = {2, 2};
		const auto values = counting(12);

		{
			mapped_image_writer writer(
				path.get(),
				make_layout(extents, 2),
				make_span(preamble.data(), preamble.size())
			);
			const auto source = make_host_array(extents, values);

			for (std::size_t i = 0; i < 3; ++i)
			{
				image_transfer_plan regions(image_transfer_shape(plane, 3, 3));
				regions.add(
					make_span(std::vector<std::size_t>{i, 0, 0}),
					make_span(std::vector<std::size_t>{i, 0, 0})
				);
				writer.write(const_array_ref(source), regions);
			}

			writer.flush();
		}

		REQUIRE( floats_of(path.get(), 12) == values );
	}

	SECTION( "what was written is there once the writer is gone" )
	{
		const std::vector<std::size_t> extents = {3, 4};
		const auto values = counting(12);

		{
			mapped_image_writer writer(
				path.get(),
				make_layout(extents, 2),
				make_span(preamble.data(), preamble.size())
			);
			const auto source = make_host_array(extents, values);
			writer.write(const_array_ref(source), whole_of(extents));
		}

		REQUIRE( floats_of(path.get(), 12) == values );
	}

	SECTION( "an empty plan writes nothing and succeeds" )
	{
		const std::vector<std::size_t> extents = {3, 4};
		mapped_image_writer writer(
			path.get(),
			make_layout(extents, 2),
			make_span(preamble.data(), preamble.size())
		);
		const auto source = make_host_array(extents, counting(12));
		const image_transfer_plan regions(image_transfer_shape(extents, 2, 2));

		REQUIRE_NOTHROW( writer.write(const_array_ref(source), regions) );
	}
}

TEST_CASE(
	"a mapped_image_writer writes its values as the layout states them",
	"[mapped_image_writer]"
)
{
	const scoped_path path("mapped_writer_layout.raw");
	const std::vector<std::size_t> extents = {3, 4};
	const auto values = counting(12);

	SECTION( "along strides that are not descending" )
	{
		// The file stores its columns one after another, so the element of
		// row r and column c goes to r + 3 c.
		{
			mapped_image_writer writer(
				path.get(),
				image_file_layout(
					make_descriptor(extents, 2),
					std::vector<std::ptrdiff_t>{1, 3},
					preamble.size(),
					get_system_byte_order()
				),
				make_span(preamble.data(), preamble.size())
			);
			const auto source = make_host_array(extents, values);
			writer.write(const_array_ref(source), whole_of(extents));
			writer.flush();
		}

		std::vector<float> expected(12);
		for (std::size_t row = 0; row < 3; ++row)
		{
			for (std::size_t column = 0; column < 4; ++column)
			{
				expected[row + 3 * column] = values[row * 4 + column];
			}
		}

		REQUIRE( floats_of(path.get(), 12) == expected );
	}

	SECTION( "in the other byte order" )
	{
		const auto other =
			get_system_byte_order() == byte_order::little_endian
				? byte_order::big_endian
				: byte_order::little_endian;

		{
			mapped_image_writer writer(
				path.get(),
				image_file_layout(
					make_descriptor(extents, 2),
					contiguous_strides(extents),
					preamble.size(),
					other
				),
				make_span(preamble.data(), preamble.size())
			);
			const auto source = make_host_array(extents, values);
			writer.write(const_array_ref(source), whole_of(extents));
			writer.flush();
		}

		auto stored = floats_of(path.get(), 12);
		auto *bytes = reinterpret_cast<char*>(stored.data());
		for (std::size_t i = 0; i < stored.size(); ++i)
		{
			std::reverse(
				bytes + i * sizeof(float),
				bytes + (i + 1) * sizeof(float)
			);
		}

		REQUIRE( stored == values );
	}

	SECTION( "converted to the data type of the file" )
	{
		const std::vector<std::size_t> square = {2, 2};
		const std::vector<float> wide = {-2.0F, -1.0F, 0.0F, 300.0F};

		{
			mapped_image_writer writer(
				path.get(),
				image_file_layout(
					make_descriptor(square, 2, numerical_type::int16),
					contiguous_strides(square),
					preamble.size(),
					get_system_byte_order()
				),
				make_span(preamble.data(), preamble.size())
			);
			const auto source = make_host_array(square, wide);
			writer.write(const_array_ref(source), whole_of(square));
			writer.flush();
		}

		const auto raw = bytes_of(path.get());
		std::vector<std::int16_t> stored(4);
		std::memcpy(
			stored.data(),
			raw.data() + preamble.size(),
			stored.size() * sizeof(std::int16_t)
		);

		REQUIRE( stored == std::vector<std::int16_t>{-2, -1, 0, 300} );
	}
}

TEST_CASE(
	"a mapped_image_writer refuses what it can not write",
	"[mapped_image_writer]"
)
{
	const scoped_path path("mapped_writer_refused.raw");
	const std::vector<std::size_t> extents = {3, 4};

	mapped_image_writer writer(
		path.get(),
		make_layout(extents, 2),
		make_span(preamble.data(), preamble.size())
	);

	SECTION( "an uninitialized source" )
	{
		REQUIRE_THROWS_AS(
			writer.write(const_array_ref(), whole_of(extents)),
			std::invalid_argument
		);
	}

	SECTION( "a region the file does not contain" )
	{
		const auto source = make_host_array(extents, counting(12));
		image_transfer_plan regions(image_transfer_shape(extents, 2, 2));
		regions.add(
			make_span(std::vector<std::size_t>{1, 0}),
			make_span(std::vector<std::size_t>{0, 0})
		);

		REQUIRE_THROWS_AS(
			writer.write(const_array_ref(source), regions),
			std::out_of_range
		);
	}
}
