// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "element_index_tags.hpp"
#include "linear_index_run.hpp"
#include "multidimensional_index.hpp"

#include <rexlib/core/layout/joint_layout.hpp>
#include <rexlib/core/platform/constexpr.hpp>

#include <cstddef>

namespace rexlib
{
namespace cpu
{
namespace detail
{

/**
 * @brief Invoke a callable with the index run of a traversal that hands out
 * no index.
 *
 * @tparam F Callable invoked as `callable(no_index_tag())`.
 * @param callable The callable.
 * @param layout The layout being traversed.
 * @param operand_count Number of operands of @p layout that are not index
 * operands.
 * @return auto What @p callable returns.
 */
template <typename F>
auto dispatch_index_run(
	F &&callable,
	const joint_layout &layout,
	std::size_t operand_count,
	no_index_tag
);

/**
 * @brief Invoke a callable with the index run of a traversal that hands out
 * the linear index.
 *
 * The change of the linear index along the innermost axis is resolved to
 * `contiguous_stride_tag` when it is one, and left as a runtime
 * `std::ptrdiff_t` otherwise.
 *
 * @tparam F Callable invoked as `callable(linear_index_run<Stride>)`, once per
 * resolved stride type.
 * @param callable The callable.
 * @param layout The layout being traversed. Its last operand must be the
 * linear index operand.
 * @param operand_count Number of operands of @p layout that precede the
 * linear index operand.
 * @return auto What @p callable returns.
 */
template <typename F>
auto dispatch_index_run(
	F &&callable,
	const joint_layout &layout,
	std::size_t operand_count,
	linear_index_tag
);

/**
 * @brief Invoke a callable with the index run of a traversal that hands out
 * the coordinates of every element.
 *
 * @tparam F Callable invoked as `callable(multidimensional_index)`.
 * @param callable The callable.
 * @param layout The layout being traversed. Every operand past the first
 * @p operand_count must be the index operand of one axis, in axis order.
 * @param operand_count Number of operands of @p layout that precede the index
 * operands.
 * @return auto What @p callable returns.
 */
template <typename F>
auto dispatch_index_run(
	F &&callable,
	const joint_layout &layout,
	std::size_t operand_count,
	multidimensional_index_tag
);

/**
 * @brief Place the index run of a traversal handing out no index at a
 * cursor.
 *
 * @param run The run.
 * @param index_offsets Offsets of the cursor past its last operand.
 * @return no_index_tag The run.
 */
REXLIB_CONSTEXPR
no_index_tag rebase_index_run(
	no_index_tag run,
	const std::ptrdiff_t *index_offsets
) noexcept;

/**
 * @brief Place a linear index run at the element a cursor points to.
 *
 * @tparam Stride Change of the linear index along the innermost axis.
 * @param run The run.
 * @param index_offsets Offsets of the cursor, starting at the linear index
 * operand.
 * @return linear_index_run<Stride> The run at that element.
 */
template <typename Stride>
REXLIB_CONSTEXPR
linear_index_run<Stride> rebase_index_run(
	const linear_index_run<Stride> &run,
	const std::ptrdiff_t *index_offsets
) noexcept;

/**
 * @brief Place a multidimensional index at the element a cursor points to.
 *
 * @param run The index.
 * @param index_offsets Offsets of the cursor, starting at the index operand
 * of the first axis.
 * @return multidimensional_index The index of that element.
 */
REXLIB_CONSTEXPR
multidimensional_index rebase_index_run(
	const multidimensional_index &run,
	const std::ptrdiff_t *index_offsets
) noexcept;

/**
 * @brief Advance the index run of a traversal handing out no index.
 *
 * @param run The run.
 * @param count Number of elements to advance by.
 * @return no_index_tag The run.
 */
REXLIB_CONSTEXPR
no_index_tag advance_index_run(no_index_tag run, std::size_t count) noexcept;

/**
 * @brief Advance a linear index run along the innermost axis.
 *
 * @tparam Stride Change of the linear index along the innermost axis.
 * @param run The run.
 * @param count Number of elements to advance by.
 * @return linear_index_run<Stride> The run at that element.
 */
template <typename Stride>
REXLIB_CONSTEXPR
linear_index_run<Stride> advance_index_run(
	const linear_index_run<Stride> &run,
	std::size_t count
) noexcept;

/**
 * @brief Advance a multidimensional index along the innermost axis.
 *
 * @param run The index.
 * @param count Number of elements to advance by.
 * @return multidimensional_index The index of that element.
 */
REXLIB_CONSTEXPR
multidimensional_index advance_index_run(
	const multidimensional_index &run,
	std::size_t count
) noexcept;

/**
 * @brief Get what a kernel is handed for a traversal handing out no index.
 *
 * @param run The run.
 * @return no_index_tag Nothing to hand over.
 */
REXLIB_CONSTEXPR no_index_tag get_kernel_index(no_index_tag run) noexcept;

/**
 * @brief Get the linear index a kernel is handed.
 *
 * @tparam Stride Change of the linear index along the innermost axis.
 * @param run The run.
 * @return std::size_t The linear index of the element the run is at.
 */
template <typename Stride>
REXLIB_CONSTEXPR
std::size_t get_kernel_index(const linear_index_run<Stride> &run) noexcept;

/**
 * @brief Get the coordinates a kernel is handed.
 *
 * @param run The index.
 * @return multidimensional_index The index itself.
 */
REXLIB_CONSTEXPR
multidimensional_index get_kernel_index(
	const multidimensional_index &run
) noexcept;

/**
 * @brief Invoke a callable that is handed no index.
 *
 * @tparam F Callable invoked as `callable(args...)`.
 * @tparam Args Types of the arguments.
 * @param callable The callable.
 * @param index Nothing to hand over.
 * @param args The arguments.
 */
template <typename F, typename... Args>
void invoke_with_index(F &&callable, no_index_tag index, Args &&...args);

/**
 * @brief Invoke a callable, handing it an index after its arguments.
 *
 * @tparam F Callable invoked as `callable(args..., index)`.
 * @tparam Index Type of the index.
 * @tparam Args Types of the arguments.
 * @param callable The callable.
 * @param index The index.
 * @param args The arguments.
 */
template <typename F, typename Index, typename... Args>
void invoke_with_index(F &&callable, const Index &index, Args &&...args);

} // namespace detail
} // namespace cpu
} // namespace rexlib

#include "element_index.inl"
