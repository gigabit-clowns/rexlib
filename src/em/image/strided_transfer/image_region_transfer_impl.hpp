// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "image_region_transfer.hpp"

#include <backends/cpu/load_store.hpp>
#include <backends/cpu/loops/elementwise_loop.hpp>
#include <rexlib/core/exceptions/unsupported_operation_error.hpp>
#include <rexlib/core/memory/byte_order.hpp>
#include <rexlib/core/numerical/numerical_type_dispatch.hpp>
#include <rexlib/core/platform/attributes.hpp>

#include <type_traits>

namespace rexlib
{
namespace em
{

/**
 * @brief Read one element of a file into an array.
 *
 * Every kernel here names its destination first, which is the order the
 * layout its loop walks names its operands in.
 *
 * Conversion goes through @ref cpu::cast rather than @ref numerical_cast:
 * the two implement the same cases, but numerical_cast reaches
 * @ref float16_t through constructors exported across the shared object
 * boundary, which is a call per element that no vectorizer gets through.
 */
struct region_read_kernel
{
	template <typename T, typename Q>
	void operator()(T *array, const Q *file) const noexcept
	{
		cpu::cast(array, file);
	}
};

/**
 * @brief Read one element of a file of the other byte order into an array.
 *
 * The bytes are reversed in the type the file holds them in, before the
 * conversion. Reversing a value of the array's type instead would be a
 * different number whenever the two widths differ.
 */
struct byte_swapped_region_read_kernel
{
	template <typename T, typename Q>
	void operator()(T *array, const Q *file) const noexcept
	{
		const auto value = reverse_byte_order(*file);
		cpu::cast(array, &value);
	}
};

/**
 * @brief Write one element of an array into a file.
 *
 * @see region_read_kernel
 */
struct region_write_kernel
{
	template <typename Q, typename T>
	void operator()(Q *file, const T *array) const noexcept
	{
		cpu::cast(file, array);
	}
};

/**
 * @brief Write one element of an array into a file of the other byte order.
 *
 * The conversion happens first and the bytes are reversed afterwards, so that
 * they are reversed in the type the file holds them in.
 *
 * @see byte_swapped_region_read_kernel
 */
struct byte_swapped_region_write_kernel
{
	template <typename Q, typename T>
	void operator()(Q *file, const T *array) const noexcept
	{
		Q value;
		cpu::cast(&value, array);
		*file = reverse_byte_order(value);
	}
};

/**
 * @brief Whether a conversion between two element types exists.
 *
 * Convertibility is a property of the static types with no
 * @ref numerical_type counterpart, so it is asked of them directly, as the
 * CPU copy builder asks it.
 */
template <typename Destination, typename Source>
struct region_transfer_support : std::is_convertible<Source, Destination>
{
};

/**
 * @brief Walk every region of a plan with one layout.
 *
 * Each region is the same iteration space reached through a different pair of
 * pointers, so the layout is built once and only the two bases move. The loop
 * over the regions is here rather than inside the layout for exactly that
 * reason.
 *
 * The destination comes first, in the order the layout names its operands.
 */
template <
	typename Kernel,
	typename DestinationPointer,
	typename SourcePointer
>
void run_regions(
	const Kernel &kernel,
	const joint_layout &layout,
	span<const std::ptrdiff_t> destination_offsets,
	span<const std::ptrdiff_t> source_offsets,
	DestinationPointer destination_data,
	SourcePointer source_data
)
{
	for (std::size_t i = 0; i < destination_offsets.size(); ++i)
	{
		cpu::run_elementwise_loop(
			kernel,
			layout,
			destination_data + destination_offsets[i],
			source_data + source_offsets[i]
		);
	}
}

template <
	typename Kernel,
	typename DestinationPointer,
	typename SourcePointer
>
void run_supported_regions(
	std::true_type,
	const Kernel &kernel,
	const joint_layout &layout,
	span<const std::ptrdiff_t> destination_offsets,
	span<const std::ptrdiff_t> source_offsets,
	DestinationPointer destination_data,
	SourcePointer source_data
)
{
	run_regions(
		kernel,
		layout,
		destination_offsets,
		source_offsets,
		destination_data,
		source_data
	);
}

// The unsupported overload never instantiates a loop, which is what keeps one
// from being compiled for every pair of element types no conversion joins.
template <
	typename Kernel,
	typename DestinationPointer,
	typename SourcePointer
>
REXLIB_NORETURN
void run_supported_regions(
	std::false_type,
	const Kernel &,
	const joint_layout &,
	span<const std::ptrdiff_t>,
	span<const std::ptrdiff_t>,
	DestinationPointer,
	SourcePointer
)
{
	throw unsupported_operation_error(
		"image_region_transfer: The values of the file can not be converted "
		"into the data type asked for."
	);
}

template <typename Q>
void read_regions_as(
	const image_region_read_plan &plan,
	std::size_t first_region,
	std::size_t region_count,
	void *array_data,
	numerical_type array_type,
	const Q *file_data,
	bool swapped
)
{
	const auto &offsets = plan.get_offsets();

	// The run is taken here rather than inside the loop, so that what walks
	// the regions is the same loop whether it is handed every region or a
	// step of them.
	const auto array_offsets =
		make_span(offsets.get_array().data() + first_region, region_count);
	const auto file_offsets =
		make_span(offsets.get_file().data() + first_region, region_count);

	dispatch_numerical_types(
		[&plan, array_offsets, file_offsets, array_data, file_data, swapped]
		(auto array_tag)
		{
			using T = typename decltype(array_tag)::type;
			const auto support = region_transfer_support<T, Q>();
			auto *array = static_cast<T*>(array_data);

			// The array is the destination here, so it comes first.
			if (swapped)
			{
				run_supported_regions(
					support,
					byte_swapped_region_read_kernel(),
					plan.get_layout(),
					array_offsets,
					file_offsets,
					array,
					file_data
				);
			}
			else
			{
				run_supported_regions(
					support,
					region_read_kernel(),
					plan.get_layout(),
					array_offsets,
					file_offsets,
					array,
					file_data
				);
			}
		},
		array_type
	);
}

template <typename Q>
void write_regions_as(
	const image_region_write_plan &plan,
	const void *array_data,
	numerical_type array_type,
	Q *file_data,
	bool swapped
)
{
	const auto &offsets = plan.get_offsets();

	dispatch_numerical_types(
		[&offsets, &plan, array_data, file_data, swapped] (auto array_tag)
		{
			using T = typename decltype(array_tag)::type;
			const auto support = region_transfer_support<Q, T>();
			const auto *array = static_cast<const T*>(array_data);

			// The file is the destination here, so it comes first.
			if (swapped)
			{
				run_supported_regions(
					support,
					byte_swapped_region_write_kernel(),
					plan.get_layout(),
					offsets.get_file(),
					offsets.get_array(),
					file_data,
					array
				);
			}
			else
			{
				run_supported_regions(
					support,
					region_write_kernel(),
					plan.get_layout(),
					offsets.get_file(),
					offsets.get_array(),
					file_data,
					array
				);
			}
		},
		array_type
	);
}

/**
 * @brief Instantiate the region transfer for one element type of a file.
 *
 * Write it once in a translation unit of its own per element type. The array
 * side is a grid over every data type there is, each cell specialized on the
 * strides of both operands, so one element type already costs about what the
 * CPU copy builder costs.
 */
#define REXLIB_INSTANTIATE_IMAGE_REGION_TRANSFER(...) \
	template void read_regions_as<__VA_ARGS__>( \
		const image_region_read_plan&, \
		std::size_t, \
		std::size_t, \
		void*, \
		numerical_type, \
		const __VA_ARGS__*, \
		bool \
	); \
	template void write_regions_as<__VA_ARGS__>( \
		const image_region_write_plan&, \
		const void*, \
		numerical_type, \
		__VA_ARGS__*, \
		bool \
	)

} // namespace em
} // namespace rexlib
