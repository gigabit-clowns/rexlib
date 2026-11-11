// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/span.hpp>

namespace rexlib
{
namespace cpu
{
namespace detail
{

/**
 * @brief Ask a type dispatch policy whether it supports some operand types.
 *
 * Kept as an indirection so that a builder can be given a type dispatch that
 * cannot answer, in which case it is assumed to support everything and an
 * unsupported combination surfaces from build() as it used to.
 *
 * @tparam Dispatch The type dispatch policy.
 */
template <typename Dispatch, typename = void>
struct type_dispatch_support_query
{
	static bool is_supported(
		span<const numerical_type>,
		span<const numerical_type>,
		span<const numerical_type>
	) noexcept
	{
		return true;
	}
};

template <typename Dispatch>
struct type_dispatch_support_query<
	Dispatch,
	decltype(void(&Dispatch::is_supported))
>
{
	static bool is_supported(
		span<const numerical_type> output_types,
		span<const numerical_type> input_types,
		span<const numerical_type> operation_data_types
	) noexcept
	{
		return Dispatch::is_supported(
			output_types,
			input_types,
			operation_data_types
		);
	}
};

} // namespace detail
} // namespace cpu
} // namespace rexlib
