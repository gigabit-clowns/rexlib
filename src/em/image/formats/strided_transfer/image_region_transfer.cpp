// SPDX-License-Identifier: GPL-3.0-only

#include "image_region_transfer.hpp"

#include <rexlib/core/exceptions/unsupported_operation_error.hpp>
#include <rexlib/core/numerical/fixed_width_float.hpp>

#include <complex>
#include <cstdint>

namespace rexlib
{
namespace em
{

// What read_regions_as and write_regions_as are defined by lives in
// image_region_transfer_impl.hpp and is instantiated once per element type, in
// the image_region_transfer_<type>.cpp files. Only the dispatch onto them is
// here.

// Every data type files are transferred in: the integers, the floating
// point numbers and the complex ones. A boolean and a character are left
// out, since neither is what a sample of an image is held as.
#define REXLIB_IMAGE_REGION_FILE_TYPES(visit) \
	visit(int8, std::int8_t); \
	visit(uint8, std::uint8_t); \
	visit(int16, std::int16_t); \
	visit(uint16, std::uint16_t); \
	visit(int32, std::int32_t); \
	visit(uint32, std::uint32_t); \
	visit(int64, std::int64_t); \
	visit(uint64, std::uint64_t); \
	visit(float16, float16_t); \
	visit(float32, float32_t); \
	visit(float64, float64_t); \
	visit(complex_float16, std::complex<float16_t>); \
	visit(complex_float32, std::complex<float32_t>); \
	visit(complex_float64, std::complex<float64_t>)

namespace
{

REXLIB_NORETURN
void reject_file_type()
{
	throw unsupported_operation_error(
		"image_region_transfer: Files are not transferred in that data type."
	);
}

} // anonymous namespace

void read_regions(
	const image_region_read_walk &walk,
	void *array_data,
	numerical_type array_type,
	const byte *file_data,
	numerical_type file_type,
	byte_order file_order
)
{
	read_regions(
		walk,
		0,
		walk.get_offsets().get_region_count(),
		array_data,
		array_type,
		file_data,
		file_type,
		file_order
	);
}

void read_regions(
	const image_region_read_walk &walk,
	std::size_t first_region,
	std::size_t region_count,
	void *array_data,
	numerical_type array_type,
	const byte *file_data,
	numerical_type file_type,
	byte_order file_order
)
{
	const auto swapped = file_order != get_system_byte_order();

	switch (file_type)
	{
	#define REXLIB_IMAGE_REGION_READ_CASE(name, ...) \
		case numerical_type::name: \
			read_regions_as( \
				walk, \
				first_region, \
				region_count, \
				array_data, \
				array_type, \
				reinterpret_cast<const __VA_ARGS__*>(file_data), \
				swapped \
			); \
			break

	REXLIB_IMAGE_REGION_FILE_TYPES(REXLIB_IMAGE_REGION_READ_CASE);

	#undef REXLIB_IMAGE_REGION_READ_CASE

	default:
		reject_file_type();
	}
}

void write_regions(
	const image_region_write_walk &walk,
	const void *array_data,
	numerical_type array_type,
	byte *file_data,
	numerical_type file_type,
	byte_order file_order
)
{
	const auto swapped = file_order != get_system_byte_order();

	switch (file_type)
	{
	#define REXLIB_IMAGE_REGION_WRITE_CASE(name, ...) \
		case numerical_type::name: \
			write_regions_as( \
				walk, \
				array_data, \
				array_type, \
				reinterpret_cast<__VA_ARGS__*>(file_data), \
				swapped \
			); \
			break

	REXLIB_IMAGE_REGION_FILE_TYPES(REXLIB_IMAGE_REGION_WRITE_CASE);

	#undef REXLIB_IMAGE_REGION_WRITE_CASE

	default:
		reject_file_type();
	}
}

#undef REXLIB_IMAGE_REGION_FILE_TYPES

} // namespace em
} // namespace rexlib
