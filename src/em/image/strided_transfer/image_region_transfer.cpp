// SPDX-License-Identifier: GPL-3.0-only

#include "image_region_transfer.hpp"

#include <rexlib/core/exceptions/invalid_operation_error.hpp>
#include <rexlib/core/numerical/fixed_width_float.hpp>

#include <complex>

namespace rexlib
{
namespace em
{

// What read_regions_as and write_regions_as are defined by lives in
// image_region_transfer_impl.hpp and is instantiated once per element type, in
// the image_region_transfer_<type>.cpp files. Only the dispatch onto them is
// here.

// Every data type files are transferred in, which is a much shorter list than
// every data type there is: the array side is dispatched over all of them,
// the file side only over these.
#define REXLIB_IMAGE_REGION_FILE_TYPES(visit) \
	visit(int8, std::int8_t); \
	visit(uint8, std::uint8_t); \
	visit(int16, std::int16_t); \
	visit(uint16, std::uint16_t); \
	visit(float16, float16_t); \
	visit(float32, float32_t); \
	visit(complex_float32, std::complex<float32_t>)

namespace
{

REXLIB_NORETURN
void reject_file_type()
{
	throw invalid_operation_error(
		"image_region_transfer: Files are not transferred in that data type."
	);
}

} // anonymous namespace

void read_regions(
	const image_region_read_plan &plan,
	void *array_data,
	numerical_type array_type,
	const byte *file_data,
	numerical_type file_type,
	byte_order file_order
)
{
	read_regions(
		plan,
		0,
		plan.get_offsets().get_region_count(),
		array_data,
		array_type,
		file_data,
		file_type,
		file_order
	);
}

void read_regions(
	const image_region_read_plan &plan,
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
				plan, \
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
	const image_region_write_plan &plan,
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
				plan, \
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
