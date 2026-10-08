// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/formats/strided_transfer/image_region_transfer.hpp>

#include <em/image/formats/strided_transfer/image_region_read_walk.hpp>
#include <em/image/formats/strided_transfer/image_region_write_walk.hpp>

#include <rexlib/core/exceptions/unsupported_operation_error.hpp>
#include <rexlib/core/layout/joint_layout_builder.hpp>
#include <rexlib/core/numerical/numerical_type_traits.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>

#include <algorithm>
#include <complex>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

// Reverses the bytes of a value without going through the production helper,
// so that a test of the byte order does not agree with it by construction.
template <typename T>
T reversed(T value)
{
	unsigned char raw[sizeof(T)];
	std::memcpy(raw, &value, sizeof(raw));
	std::reverse(raw, raw + sizeof(raw));

	T result;
	std::memcpy(&result, raw, sizeof(result));
	return result;
}

// A complex value is two values side by side, each reversed on its own.
template <typename T>
std::complex<T> reversed(const std::complex<T> &value)
{
	return std::complex<T>(reversed(value.real()), reversed(value.imag()));
}

template <typename T>
std::vector<T> counting(std::size_t count, T first = T())
{
	std::vector<T> values(count);
	for (std::size_t i = 0; i < count; ++i)
	{
		values[i] = static_cast<T>(first + static_cast<T>(i));
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

byte_order other_byte_order()
{
	return get_system_byte_order() == byte_order::little_endian
		? byte_order::big_endian
		: byte_order::little_endian;
}

template <typename T>
const rexlib::byte* as_file(const std::vector<T> &values)
{
	return reinterpret_cast<const rexlib::byte*>(values.data());
}

template <typename T>
rexlib::byte* as_file(std::vector<T> &values)
{
	return reinterpret_cast<rexlib::byte*>(values.data());
}

template <typename T>
bool same_bytes(const std::vector<T> &lhs, const std::vector<T> &rhs)
{
	return
		lhs.size() == rhs.size() &&
		std::memcmp(lhs.data(), rhs.data(), lhs.size() * sizeof(T)) == 0;
}

// Writes four values into a file of their own type and reads them back, in
// a given byte order, and checks both what the file holds and what returns.
template <typename T>
void check_held_in(const std::vector<T> &values, byte_order order)
{
	const std::vector<std::size_t> extents = {2, 2};
	const auto strides = contiguous_strides(extents);
	const auto type = numerical_type_of<T>::value;

	image_transfer_plan regions(image_transfer_shape(extents, 2, 2));
	regions.add(make_span(std::vector<std::size_t>{0, 0}),
		make_span(std::vector<std::size_t>{0, 0}));

	std::vector<T> file(values.size());
	const image_region_write_walk writer(
		regions,
		make_span(extents), make_span(strides),
		make_span(extents), make_span(strides),
		0
	);
	write_regions(
		writer, values.data(), type, as_file(file), type, order);

	std::vector<T> expected = values;
	if (order != get_system_byte_order())
	{
		std::transform(values.begin(), values.end(), expected.begin(),
			[] (const T &value) { return reversed(value); });
	}

	REQUIRE( same_bytes(file, expected) );

	std::vector<T> reread(values.size());
	const image_region_read_walk reader(
		regions,
		make_span(extents), make_span(strides),
		make_span(extents), make_span(strides),
		0
	);
	read_regions(
		reader, reread.data(), type, as_file(file), type, order);

	REQUIRE( same_bytes(reread, values) );
}

template <typename T>
void check_held(const std::vector<T> &values)
{
	check_held_in(values, get_system_byte_order());
	check_held_in(values, other_byte_order());
}

} // anonymous namespace

TEST_CASE( "a file holds every integer, floating point and complex type",
	"[image_region_transfer]" )
{
	SECTION( "integers of one byte" )
	{
		check_held<std::int8_t>({-128, -1, 0, 127});
		check_held<std::uint8_t>({0, 1, 128, 255});
	}

	SECTION( "integers of two bytes" )
	{
		check_held<std::int16_t>({-32768, -2, 258, 32767});
		check_held<std::uint16_t>({0, 258, 32768, 65535});
	}

	SECTION( "integers of four bytes" )
	{
		check_held<std::int32_t>({-2147483647 - 1, -2, 16909060, 2147483647});
		check_held<std::uint32_t>({0U, 16909060U, 2147483648U, 4294967295U});
	}

	SECTION( "integers of eight bytes" )
	{
		check_held<std::int64_t>({
			-9223372036854775807LL - 1, -2, 72623859790382856LL,
			9223372036854775807LL
		});
		check_held<std::uint64_t>({
			0ULL, 72623859790382856ULL, 9223372036854775808ULL,
			18446744073709551615ULL
		});
	}

	SECTION( "floating point numbers" )
	{
		check_held<float16_t>({
			float16_t(0.5F), float16_t(-1.5F), float16_t(0.0F),
			float16_t(1024.0F)
		});
		check_held<float32_t>({0.5F, -1.5F, 0.0F, 16777216.0F});
		check_held<float64_t>({0.5, -1.5, 0.0, 9007199254740993.0});
	}

	SECTION( "complex numbers" )
	{
		check_held<std::complex<float16_t>>({
			{float16_t(0.5F), float16_t(-1.5F)},
			{float16_t(0.0F), float16_t(1.0F)},
			{float16_t(3.0F), float16_t(4.0F)},
			{float16_t(-1.0F), float16_t(0.25F)}
		});
		check_held<std::complex<float32_t>>({
			{0.5F, -1.5F}, {0.0F, 1.0F}, {3.0F, 4.0F}, {-1.0F, 0.25F}
		});
		check_held<std::complex<float64_t>>({
			{0.5, -1.5}, {0.0, 1.0}, {3.0, 4.0}, {-1.0, 0.25}
		});
	}
}

TEST_CASE( "the operand named first is the one the axes are ordered for",
	"[image_region_transfer]" )
{
	// What the direction of a transfer is for. joint_layout compares the
	// strides of its operands in the order they were added and takes the
	// first answer that is not a tie, so naming the destination first is what
	// makes the traversal sequential in the side being written.
	const std::vector<std::size_t> extents = {4, 8};
	const std::vector<std::ptrdiff_t> row_major = {8, 1};
	const std::vector<std::ptrdiff_t> column_major = {1, 4};

	joint_layout_builder row_first;
	row_first.set_extents(make_span(extents));
	row_first.add_operand(make_span(extents), make_span(row_major), 0);
	row_first.add_operand(make_span(extents), make_span(column_major), 0);
	const auto ordered_for_rows = row_first.build();

	joint_layout_builder column_first;
	column_first.set_extents(make_span(extents));
	column_first.add_operand(make_span(extents), make_span(column_major), 0);
	column_first.add_operand(make_span(extents), make_span(row_major), 0);
	const auto ordered_for_columns = column_first.build();

	// Neither operand is contiguous in the other's order, so nothing
	// coalesces and both layouts keep their two axes. The innermost axis is
	// the one the first operand walks with a stride of one.
	REQUIRE( ordered_for_rows.get_strides(0)[0] == 1 );
	REQUIRE( ordered_for_columns.get_strides(0)[0] == 1 );
	REQUIRE( ordered_for_rows.get_strides(1)[0] != 1 );
	REQUIRE( ordered_for_columns.get_strides(1)[0] != 1 );
}

TEST_CASE( "one region is moved out of a file and into an array",
	"[image_region_transfer]" )
{
	const std::vector<std::size_t> extents = {3, 4};
	const auto strides = contiguous_strides(extents);
	const auto file = counting<float32_t>(12);

	image_transfer_plan regions(image_transfer_shape(extents, 2, 2));
	regions.add(make_span(std::vector<std::size_t>{0, 0}),
		make_span(std::vector<std::size_t>{0, 0}));

	const image_region_read_walk plan(
		regions,
		make_span(extents), make_span(strides),
		make_span(extents), make_span(strides),
		0
	);

	SECTION( "every value arrives where it belongs" )
	{
		std::vector<float32_t> array(12, -1.0F);
		read_regions(
			plan,
			array.data(), numerical_type::float32,
			as_file(file), numerical_type::float32,
			get_system_byte_order()
		);

		REQUIRE( array == file );
	}

	SECTION( "the transfer reports how many regions it holds" )
	{
		REQUIRE( plan.get_offsets().get_region_count() == 1 );
	}
}

TEST_CASE( "a batch of regions shares one layout",
	"[image_region_transfer]" )
{
	const std::vector<std::size_t> file_extents = {3, 2, 2};
	const std::vector<std::size_t> region_extents = {2, 2};
	const auto file_strides = contiguous_strides(file_extents);
	const auto file = counting<float32_t>(12);

	image_transfer_plan regions(image_transfer_shape(region_extents, 3, 3));
	for (std::size_t i = 0; i < 3; ++i)
	{
		regions.add(
			make_span(std::vector<std::size_t>{i, 0, 0}),
			make_span(std::vector<std::size_t>{2 - i, 0, 0})
		);
	}

	const image_region_read_walk plan(
		regions,
		make_span(file_extents), make_span(file_strides),
		make_span(file_extents), make_span(file_strides),
		0
	);

	SECTION( "each region lands where its own offsets put it" )
	{
		std::vector<float32_t> array(12, -1.0F);
		read_regions(
			plan,
			array.data(), numerical_type::float32,
			as_file(file), numerical_type::float32,
			get_system_byte_order()
		);

		const std::vector<float32_t> expected = {
			8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3
		};

		REQUIRE( array == expected );
		REQUIRE( plan.get_offsets().get_region_count() == 3 );
	}
}

TEST_CASE( "a run of the regions of a batch is moved on its own",
	"[image_region_transfer]" )
{
	const std::vector<std::size_t> file_extents = {3, 2, 2};
	const std::vector<std::size_t> region_extents = {2, 2};
	const auto file_strides = contiguous_strides(file_extents);
	const auto file = counting<float32_t>(12);

	// Each region lands where it sits in the file, so a run of them leaves
	// the slots of the others alone.
	image_transfer_plan regions(image_transfer_shape(region_extents, 3, 3));
	for (std::size_t i = 0; i < 3; ++i)
	{
		regions.add(
			make_span(std::vector<std::size_t>{i, 0, 0}),
			make_span(std::vector<std::size_t>{i, 0, 0})
		);
	}

	const image_region_read_walk plan(
		regions,
		make_span(file_extents), make_span(file_strides),
		make_span(file_extents), make_span(file_strides),
		0
	);

	SECTION( "only the regions of the run are moved" )
	{
		std::vector<float32_t> array(12, -1.0F);
		read_regions(
			plan, 1, 1,
			array.data(), numerical_type::float32,
			as_file(file), numerical_type::float32,
			get_system_byte_order()
		);

		const std::vector<float32_t> expected = {
			-1, -1, -1, -1, 4, 5, 6, 7, -1, -1, -1, -1
		};

		REQUIRE( array == expected );
	}

	SECTION( "the run that spans the batch moves all of it" )
	{
		std::vector<float32_t> array(12, -1.0F);
		read_regions(
			plan, 0, 3,
			array.data(), numerical_type::float32,
			as_file(file), numerical_type::float32,
			get_system_byte_order()
		);

		REQUIRE( array == file );
	}

	SECTION( "consecutive runs together move the whole batch" )
	{
		std::vector<float32_t> array(12, -1.0F);
		for (std::size_t first = 0; first < 3; ++first)
		{
			read_regions(
				plan, first, 1,
				array.data(), numerical_type::float32,
				as_file(file), numerical_type::float32,
				get_system_byte_order()
			);
		}

		REQUIRE( array == file );
	}

	SECTION( "a run of no region moves nothing" )
	{
		std::vector<float32_t> array(12, -1.0F);
		read_regions(
			plan, 1, 0,
			array.data(), numerical_type::float32,
			as_file(file), numerical_type::float32,
			get_system_byte_order()
		);

		REQUIRE( array == std::vector<float32_t>(12, -1.0F) );
	}
}

TEST_CASE( "a region reaches an array of a different rank",
	"[image_region_transfer]" )
{
	const std::vector<std::size_t> file_extents = {3, 2, 2};
	const std::vector<std::size_t> array_extents = {2, 2};
	const std::vector<std::size_t> region_extents = {2, 2};
	const auto file_strides = contiguous_strides(file_extents);
	const auto array_strides = contiguous_strides(array_extents);
	const auto file = counting<float32_t>(12);

	image_transfer_plan regions(image_transfer_shape(region_extents, 3, 2));
	regions.add(make_span(std::vector<std::size_t>{1, 0, 0}),
		make_span(std::vector<std::size_t>{0, 0}));

	const image_region_read_walk plan(
		regions,
		make_span(file_extents), make_span(file_strides),
		make_span(array_extents), make_span(array_strides),
		0
	);

	std::vector<float32_t> array(4, -1.0F);
	read_regions(
			plan,
		array.data(), numerical_type::float32,
		as_file(file), numerical_type::float32,
		get_system_byte_order()
	);

	REQUIRE( array == std::vector<float32_t>{4, 5, 6, 7} );
}

TEST_CASE( "a region lands in a strided array",
	"[image_region_transfer]" )
{
	const std::vector<std::size_t> extents = {2, 3};
	const std::vector<std::ptrdiff_t> file_strides = {3, 1};
	// A (2,3) window of a (2,5) buffer, starting one element in.
	const std::vector<std::ptrdiff_t> array_strides = {5, 1};
	const auto file = counting<float32_t>(6);

	image_transfer_plan regions(image_transfer_shape(extents, 2, 2));
	regions.add(make_span(std::vector<std::size_t>{0, 0}),
		make_span(std::vector<std::size_t>{0, 0}));

	const image_region_read_walk plan(
		regions,
		make_span(extents), make_span(file_strides),
		make_span(extents), make_span(array_strides),
		1
	);

	std::vector<float32_t> array(10, -1.0F);
	read_regions(
			plan,
		array.data(), numerical_type::float32,
		as_file(file), numerical_type::float32,
		get_system_byte_order()
	);

	const std::vector<float32_t> expected = {
		-1, 0, 1, 2, -1,
		-1, 3, 4, 5, -1
	};

	REQUIRE( array == expected );
}

TEST_CASE( "values are converted into the type asked for",
	"[image_region_transfer]" )
{
	const std::vector<std::size_t> extents = {2, 2};
	const auto strides = contiguous_strides(extents);

	image_transfer_plan regions(image_transfer_shape(extents, 2, 2));
	regions.add(make_span(std::vector<std::size_t>{0, 0}),
		make_span(std::vector<std::size_t>{0, 0}));

	const image_region_read_walk plan(
		regions,
		make_span(extents), make_span(strides),
		make_span(extents), make_span(strides),
		0
	);

	SECTION( "a narrower integer widens into a float" )
	{
		const std::vector<std::int16_t> file = {-2, -1, 0, 1};
		std::vector<float32_t> array(4, 9.0F);

		read_regions(
			plan,
			array.data(), numerical_type::float32,
			as_file(file), numerical_type::int16,
			get_system_byte_order()
		);

		REQUIRE( array == std::vector<float32_t>{-2, -1, 0, 1} );
	}

	SECTION( "half precision widens into a double" )
	{
		// Every one of these is exact in half precision, so widening them
		// loses nothing and the comparison can be an equality.
		const std::vector<float16_t> file = {
			float16_t(0.5F), float16_t(1.5F),
			float16_t(-2.5F), float16_t(3.0F)
		};
		std::vector<float64_t> array(4, 9.0);

		read_regions(
			plan,
			array.data(), numerical_type::float64,
			as_file(file), numerical_type::float16,
			get_system_byte_order()
		);

		REQUIRE( array == std::vector<float64_t>{0.5, 1.5, -2.5, 3.0} );
	}

	SECTION( "a double narrows into half precision" )
	{
		const std::vector<float64_t> array = {0.5, 1.5, -2.5, 3.0};
		std::vector<float16_t> file(4);

		const image_region_write_walk writer(
			regions,
			make_span(extents), make_span(strides),
			make_span(extents), make_span(strides),
			0
		);
		write_regions(
			writer,
			array.data(), numerical_type::float64,
			as_file(file), numerical_type::float16,
			get_system_byte_order()
		);

		REQUIRE( static_cast<float>(file[0]) == 0.5F );
		REQUIRE( static_cast<float>(file[2]) == -2.5F );
	}

	SECTION( "a real value becomes the real part of a complex one" )
	{
		const std::vector<float32_t> file = {1, 2, 3, 4};
		std::vector<std::complex<float32_t>> array(4);

		read_regions(
			plan,
			array.data(), numerical_type::complex_float32,
			as_file(file), numerical_type::float32,
			get_system_byte_order()
		);

		REQUIRE( array[0] == std::complex<float32_t>(1.0F, 0.0F) );
		REQUIRE( array[3] == std::complex<float32_t>(4.0F, 0.0F) );
	}

	SECTION( "a complex value has no real destination" )
	{
		const std::vector<std::complex<float32_t>> file(4);
		std::vector<float32_t> array(4);

		REQUIRE_THROWS_AS(
			read_regions(
			plan,
				array.data(), numerical_type::float32,
				as_file(file), numerical_type::complex_float32,
				get_system_byte_order()
			),
			unsupported_operation_error
		);
	}

	SECTION( "a file data type no transfer moves is refused" )
	{
		const std::vector<std::uint8_t> file(4);
		std::vector<std::uint8_t> array(4);

		REQUIRE_THROWS_AS(
			read_regions(
			plan,
				array.data(), numerical_type::uint8,
				as_file(file), numerical_type::boolean,
				get_system_byte_order()
			),
			unsupported_operation_error
		);
		REQUIRE_THROWS_AS(
			read_regions(
			plan,
				array.data(), numerical_type::uint8,
				as_file(file), numerical_type::char8,
				get_system_byte_order()
			),
			unsupported_operation_error
		);
	}
}

TEST_CASE( "a file of the other byte order is read in it",
	"[image_region_transfer]" )
{
	const std::vector<std::size_t> extents = {2, 2};
	const auto strides = contiguous_strides(extents);

	image_transfer_plan regions(image_transfer_shape(extents, 2, 2));
	regions.add(make_span(std::vector<std::size_t>{0, 0}),
		make_span(std::vector<std::size_t>{0, 0}));

	const image_region_read_walk plan(
		regions,
		make_span(extents), make_span(strides),
		make_span(extents), make_span(strides),
		0
	);

	SECTION( "reversed integers arrive as themselves" )
	{
		const std::vector<std::int16_t> values = {-2, -1, 3, 1000};
		std::vector<std::int16_t> file(values.size());
		std::transform(values.begin(), values.end(), file.begin(),
			[] (std::int16_t v) { return reversed(v); });

		std::vector<std::int16_t> array(4, 0);
		read_regions(
			plan,
			array.data(), numerical_type::int16,
			as_file(file), numerical_type::int16,
			other_byte_order()
		);

		REQUIRE( array == values );
	}

	SECTION( "reversed floats arrive as themselves" )
	{
		const std::vector<float32_t> values = {1.5F, -2.25F, 0.0F, 1024.0F};
		std::vector<float32_t> file(values.size());
		std::transform(values.begin(), values.end(), file.begin(),
			[] (float32_t v) { return reversed(v); });

		std::vector<float32_t> array(4, 0.0F);
		read_regions(
			plan,
			array.data(), numerical_type::float32,
			as_file(file), numerical_type::float32,
			other_byte_order()
		);

		REQUIRE( array == values );
	}

	SECTION( "each component of a complex value is reversed on its own" )
	{
		const std::vector<std::complex<float32_t>> values = {
			{1.5F, -2.5F}, {0.0F, 1.0F}, {3.0F, 4.0F}, {-1.0F, 0.5F}
		};
		std::vector<std::complex<float32_t>> file(values.size());
		std::transform(values.begin(), values.end(), file.begin(),
			[] (const std::complex<float32_t> &v)
			{
				return std::complex<float32_t>(
					reversed(v.real()), reversed(v.imag())
				);
			});

		std::vector<std::complex<float32_t>> array(4);
		read_regions(
			plan,
			array.data(), numerical_type::complex_float32,
			as_file(file), numerical_type::complex_float32,
			other_byte_order()
		);

		REQUIRE( array == values );
	}

	SECTION( "single bytes are the same in either order" )
	{
		const std::vector<std::int8_t> file = {-2, -1, 0, 1};
		std::vector<std::int8_t> array(4, 9);

		read_regions(
			plan,
			array.data(), numerical_type::int8,
			as_file(file), numerical_type::int8,
			other_byte_order()
		);

		REQUIRE( array == file );
	}
}

TEST_CASE( "a region is moved out of an array and into a file",
	"[image_region_transfer]" )
{
	const std::vector<std::size_t> extents = {2, 2};
	const auto strides = contiguous_strides(extents);

	image_transfer_plan regions(image_transfer_shape(extents, 2, 2));
	regions.add(make_span(std::vector<std::size_t>{0, 0}),
		make_span(std::vector<std::size_t>{0, 0}));

	const image_region_write_walk plan(
		regions,
		make_span(extents), make_span(strides),
		make_span(extents), make_span(strides),
		0
	);

	SECTION( "values are converted into what the file holds" )
	{
		const std::vector<float32_t> array = {-2.0F, -1.0F, 0.0F, 1.0F};
		std::vector<std::int16_t> file(4, 9);

		write_regions(
			plan,
			array.data(), numerical_type::float32,
			as_file(file), numerical_type::int16,
			get_system_byte_order()
		);

		REQUIRE( file == std::vector<std::int16_t>{-2, -1, 0, 1} );
	}

	SECTION( "a file of the other byte order is written in it" )
	{
		const std::vector<float32_t> array = {-2.0F, -1.0F, 0.0F, 1.0F};
		std::vector<std::int16_t> file(4, 9);

		write_regions(
			plan,
			array.data(), numerical_type::float32,
			as_file(file), numerical_type::int16,
			other_byte_order()
		);

		// The bytes are reversed in the type the file holds, which is what
		// makes a wider array type land as the narrower value it converts to
		// rather than as a reversal of the original.
		const std::vector<std::int16_t> expected = {
			reversed<std::int16_t>(-2), reversed<std::int16_t>(-1),
			reversed<std::int16_t>(0), reversed<std::int16_t>(1)
		};

		REQUIRE( file == expected );
	}

	SECTION( "what is written is read back unchanged" )
	{
		const std::vector<float32_t> array = {1.5F, -2.25F, 0.0F, 8.0F};
		std::vector<float32_t> file(4, 9.0F);
		std::vector<float32_t> reread(4, 0.0F);

		write_regions(
			plan,
			array.data(), numerical_type::float32,
			as_file(file), numerical_type::float32,
			other_byte_order()
		);

		const image_region_read_walk reader(
			regions,
			make_span(extents), make_span(strides),
			make_span(extents), make_span(strides),
			0
		);
		read_regions(
			reader,
			reread.data(), numerical_type::float32,
			as_file(file), numerical_type::float32,
			other_byte_order()
		);

		REQUIRE( reread == array );
	}

	SECTION( "a type the file can not hold is refused" )
	{
		const std::vector<std::complex<float32_t>> array(4);
		std::vector<float32_t> file(4);

		REQUIRE_THROWS_AS(
			write_regions(
			plan,
				array.data(), numerical_type::complex_float32,
				as_file(file), numerical_type::float32,
				get_system_byte_order()
			),
			unsupported_operation_error
		);
	}
}

TEST_CASE( "a batch that does not fit is refused before anything moves",
	"[image_region_transfer]" )
{
	const std::vector<std::size_t> extents = {2, 2};
	const std::vector<std::size_t> region_extents = {2, 2};
	const auto strides = contiguous_strides(extents);

	SECTION( "a region past the end of the file is refused" )
	{
		image_transfer_plan regions(image_transfer_shape(region_extents, 2, 2));
		regions.add(make_span(std::vector<std::size_t>{1, 0}),
			make_span(std::vector<std::size_t>{0, 0}));

		REQUIRE_THROWS_AS(
			image_region_read_walk(
				regions,
				make_span(extents), make_span(strides),
				make_span(extents), make_span(strides),
				0
			),
			std::out_of_range
		);
	}

	SECTION( "a region past the end of the array is refused" )
	{
		image_transfer_plan regions(image_transfer_shape(region_extents, 2, 2));
		regions.add(make_span(std::vector<std::size_t>{0, 0}),
			make_span(std::vector<std::size_t>{0, 1}));

		REQUIRE_THROWS_AS(
			image_region_read_walk(
				regions,
				make_span(extents), make_span(strides),
				make_span(extents), make_span(strides),
				0
			),
			std::out_of_range
		);
	}

	SECTION( "one bad region among good ones refuses the whole batch" )
	{
		image_transfer_plan regions(image_transfer_shape(region_extents, 2, 2));
		regions.add(make_span(std::vector<std::size_t>{0, 0}),
			make_span(std::vector<std::size_t>{0, 0}));
		regions.add(make_span(std::vector<std::size_t>{0, 3}),
			make_span(std::vector<std::size_t>{0, 0}));

		REQUIRE_THROWS_AS(
			image_region_read_walk(
				regions,
				make_span(extents), make_span(strides),
				make_span(extents), make_span(strides),
				0
			),
			std::out_of_range
		);
	}
}

TEST_CASE( "a batch whose ranks disagree with its sides is refused",
	"[image_region_transfer]" )
{
	const std::vector<std::size_t> extents = {2, 2};
	const std::vector<std::size_t> deeper = {1, 2, 2};
	const auto strides = contiguous_strides(extents);
	const auto deeper_strides = contiguous_strides(deeper);

	image_transfer_plan regions(image_transfer_shape(extents, 2, 2));
	regions.add(make_span(std::vector<std::size_t>{0, 0}),
		make_span(std::vector<std::size_t>{0, 0}));

	SECTION( "file extents of the wrong rank are refused" )
	{
		REQUIRE_THROWS_AS(
			image_region_read_walk(
				regions,
				make_span(deeper), make_span(deeper_strides),
				make_span(extents), make_span(strides),
				0
			),
			std::invalid_argument
		);
	}

	SECTION( "array extents of the wrong rank are refused" )
	{
		REQUIRE_THROWS_AS(
			image_region_read_walk(
				regions,
				make_span(extents), make_span(strides),
				make_span(deeper), make_span(deeper_strides),
				0
			),
			std::invalid_argument
		);
	}

	SECTION( "strides that do not match their extents are refused" )
	{
		REQUIRE_THROWS_AS(
			image_region_read_walk(
				regions,
				make_span(extents), make_span(deeper_strides),
				make_span(extents), make_span(strides),
				0
			),
			std::invalid_argument
		);
	}
}

TEST_CASE( "an empty batch moves nothing and succeeds",
	"[image_region_transfer]" )
{
	const std::vector<std::size_t> extents = {2, 2};
	const auto strides = contiguous_strides(extents);
	const std::vector<float32_t> file = {1, 2, 3, 4};

	const image_transfer_plan regions(image_transfer_shape(extents, 2, 2));
	const image_region_read_walk plan(
		regions,
		make_span(extents), make_span(strides),
		make_span(extents), make_span(strides),
		0
	);

	std::vector<float32_t> array(4, -1.0F);
	read_regions(
			plan,
		array.data(), numerical_type::float32,
		as_file(file), numerical_type::float32,
		get_system_byte_order()
	);

	REQUIRE( plan.get_offsets().get_region_count() == 0 );
	REQUIRE( array == std::vector<float32_t>{-1, -1, -1, -1} );
}
