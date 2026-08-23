// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/numerical/numerical_type.hpp>
#include <xmipp4/core/span.hpp>

namespace rex
{
namespace cpu
{
namespace detail
{

/**
 * @brief Ask a type dispatcher whether it supports some operand types.
 *
 * Kept as an indirection so that a builder can be given a dispatcher that
 * cannot answer, in which case it is assumed to support everything and an
 * unsupported combination surfaces from build() as it used to.
 *
 * @tparam Dispatcher The type dispatcher.
 */
template <typename Dispatcher, typename = void>
struct dispatcher_support_query
{
	static bool is_supported(
		span<const numerical_type>,
		span<const numerical_type>
	) noexcept
	{
		return true;
	}
};

template <typename Dispatcher>
struct dispatcher_support_query<
	Dispatcher,
	decltype(void(&Dispatcher::is_supported))
>
{
	static bool is_supported(
		span<const numerical_type> output_types,
		span<const numerical_type> input_types
	) noexcept
	{
		return Dispatcher::is_supported(output_types, input_types);
	}
};

} // namespace detail
} // namespace cpu
} // namespace rex
