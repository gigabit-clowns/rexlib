// SPDX-License-Identifier: GPL-3.0-only

#include "element_index.hpp"

#include "inner_loop_stride_dispatch.hpp"

#include <rexlib/core/platform/assert.hpp>

#include <utility>

namespace rexlib
{
namespace cpu
{
namespace detail
{

template <typename F>
inline
auto dispatch_index_run(
	F &&callable,
	const joint_layout& /*layout*/,
	std::size_t /*operand_count*/,
	no_index_tag
)
{
	return std::forward<F>(callable)(no_index_tag());
}

template <typename F>
inline
auto dispatch_index_run(
	F &&callable,
	const joint_layout &layout,
	std::size_t operand_count,
	linear_index_tag
)
{
	REXLIB_ASSERT( layout.get_number_of_operands() == operand_count + 1 );

	// A linear index moves along every axis holding more than one element,
	// so a broadcasting tag would never be taken. A layout of no axes visits
	// a single element, for which any stride will do.
	const auto stride = layout.get_rank() == 0
		? std::ptrdiff_t(1)
		: layout.get_strides(operand_count)[0];

	if (stride == 1)
	{
		return std::forward<F>(callable)(
			linear_index_run<contiguous_stride_tag>(
				0,
				contiguous_stride_tag()
			)
		);
	}

	return std::forward<F>(callable)(
		linear_index_run<std::ptrdiff_t>(0, stride)
	);
}

template <typename F>
inline
auto dispatch_index_run(
	F &&callable,
	const joint_layout &layout,
	std::size_t operand_count,
	multidimensional_index_tag
)
{
	REXLIB_ASSERT( layout.get_number_of_operands() >= operand_count );

	const auto rank = layout.get_number_of_operands() - operand_count;
	auto inner_axis = rank;
	if (layout.get_rank() > 0)
	{
		for (std::size_t axis = 0; axis < rank; ++axis)
		{
			if (layout.get_strides(operand_count + axis)[0] != 0)
			{
				inner_axis = axis;
				break;
			}
		}
	}

	return std::forward<F>(callable)(multidimensional_index(rank, inner_axis));
}

REXLIB_INLINE_CONSTEXPR
no_index_tag rebase_index_run(
	no_index_tag run,
	const std::ptrdiff_t* /*index_offsets*/
) noexcept
{
	return run;
}

template <typename Stride>
REXLIB_INLINE_CONSTEXPR
linear_index_run<Stride> rebase_index_run(
	const linear_index_run<Stride> &run,
	const std::ptrdiff_t *index_offsets
) noexcept
{
	return run.rebased(index_offsets);
}

REXLIB_INLINE_CONSTEXPR
multidimensional_index rebase_index_run(
	const multidimensional_index &run,
	const std::ptrdiff_t *index_offsets
) noexcept
{
	return run.rebased(index_offsets);
}

REXLIB_INLINE_CONSTEXPR
no_index_tag advance_index_run(
	no_index_tag run,
	std::size_t /*count*/
) noexcept
{
	return run;
}

template <typename Stride>
REXLIB_INLINE_CONSTEXPR
linear_index_run<Stride> advance_index_run(
	const linear_index_run<Stride> &run,
	std::size_t count
) noexcept
{
	return run.advanced(count);
}

REXLIB_INLINE_CONSTEXPR
multidimensional_index advance_index_run(
	const multidimensional_index &run,
	std::size_t count
) noexcept
{
	return run.advanced(count);
}

REXLIB_INLINE_CONSTEXPR
no_index_tag get_kernel_index(no_index_tag run) noexcept
{
	return run;
}

template <typename Stride>
REXLIB_INLINE_CONSTEXPR
std::size_t get_kernel_index(const linear_index_run<Stride> &run) noexcept
{
	return run.get_index();
}

REXLIB_INLINE_CONSTEXPR
multidimensional_index get_kernel_index(
	const multidimensional_index &run
) noexcept
{
	return run;
}

template <typename F, typename... Args>
inline
void invoke_with_index(F &&callable, no_index_tag, Args &&...args)
{
	std::forward<F>(callable)(std::forward<Args>(args)...);
}

template <typename F, typename Index, typename... Args>
inline
void invoke_with_index(F &&callable, const Index &index, Args &&...args)
{
	std::forward<F>(callable)(std::forward<Args>(args)..., index);
}

} // namespace detail
} // namespace cpu
} // namespace rexlib
