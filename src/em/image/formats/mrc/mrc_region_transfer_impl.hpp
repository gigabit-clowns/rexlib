// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "mrc_region_transfer.hpp"

#include "mrc_transfer.hpp"

#include <backends/cpu/loops/elementwise_loop.hpp>
#include <rexlib/core/exceptions/invalid_operation_error.hpp>
#include <rexlib/core/numerical/numerical_type_dispatch.hpp>
#include <rexlib/core/platform/attributes.hpp>

#include <type_traits>

namespace rexlib
{
namespace em
{
namespace mrc
{
namespace detail
{

/**
 * @brief Whether a conversion between two element types exists.
 *
 * Convertibility is a property of the static types with no
 * @ref numerical_type counterpart, so it is asked of them directly, as the
 * CPU copy builder asks it.
 */
template <typename Destination, typename Source>
struct transfer_support : std::is_convertible<Source, Destination>
{
};

/**
 * @brief Walk every region of a batch with one layout.
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
	throw invalid_operation_error(
		"mrc: The values of the file can not be converted into the data "
		"type asked for."
	);
}

template <typename Q>
void read_regions_as(
	const mrc_region_read_plan &plan,
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
	// the regions is the same loop whether it is handed a whole batch or a
	// step of one.
	const auto array_offsets =
		make_span(offsets.get_array().data() + first_region, region_count);
	const auto file_offsets =
		make_span(offsets.get_file().data() + first_region, region_count);

	dispatch_numerical_types(
		[&plan, array_offsets, file_offsets, array_data, file_data, swapped]
		(auto array_tag)
		{
			using T = typename decltype(array_tag)::type;
			const auto support = transfer_support<T, Q>();
			auto *array = static_cast<T*>(array_data);

			// The array is the destination here, so it comes first.
			if (swapped)
			{
				run_supported_regions(
					support,
					mrc_byte_swapped_read_kernel(),
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
					mrc_read_kernel(),
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
	const mrc_region_write_plan &plan,
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
			const auto support = transfer_support<Q, T>();
			const auto *array = static_cast<const T*>(array_data);

			// The file is the destination here, so it comes first.
			if (swapped)
			{
				run_supported_regions(
					support,
					mrc_byte_swapped_write_kernel(),
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
					mrc_write_kernel(),
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

} // namespace detail

/**
 * @brief Instantiate the region transfer for one element type of a file.
 *
 * Write it once in a translation unit of its own per element type. The array
 * side is a grid over every data type there is, each cell specialized on the
 * strides of both operands, so one element type already costs about what the
 * CPU copy builder costs.
 */
#define REXLIB_INSTANTIATE_MRC_REGION_TRANSFER(...) \
	template void detail::read_regions_as<__VA_ARGS__>( \
		const mrc_region_read_plan&, \
		std::size_t, \
		std::size_t, \
		void*, \
		numerical_type, \
		const __VA_ARGS__*, \
		bool \
	); \
	template void detail::write_regions_as<__VA_ARGS__>( \
		const mrc_region_write_plan&, \
		const void*, \
		numerical_type, \
		__VA_ARGS__*, \
		bool \
	)

} // namespace mrc
} // namespace em
} // namespace rexlib
