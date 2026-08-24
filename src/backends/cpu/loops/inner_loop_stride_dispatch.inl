// SPDX-License-Identifier: GPL-3.0-only

#include "inner_loop_stride_dispatch.hpp"

namespace rex
{
namespace cpu
{


namespace detail
{
	
template <typename F>
REXLIB_INLINE_CONSTEXPR
auto dispatch_single_stride(F&& visitor, std::ptrdiff_t stride) 
{
	switch (stride) 
	{
	case 0: 
		return std::forward<F>(visitor)(broadcasting_stride_tag());
	case 1: 
		return std::forward<F>(visitor)(contiguous_stride_tag());
	default: 
		return std::forward<F>(visitor)(stride);
	}
}

template <typename F>
REXLIB_INLINE_CONSTEXPR
auto dispatch_strides(F&& visitor) 
{
	return std::forward<F>(visitor)();
}

template <typename F, typename... Integers>
REXLIB_INLINE_CONSTEXPR
auto dispatch_strides(
	F&& visitor, 
	std::ptrdiff_t stride, 
	Integers ...other_strides
) 
{
	return dispatch_single_stride(
		[&visitor, &other_strides...](auto tag) 
		{
			const auto bound_visitor = [tag, &visitor](auto... other_tags) 
			{
   				return std::forward<F>(visitor)(tag, other_tags...);
			};

			return dispatch_strides(
				bound_visitor,
				other_strides...
			);
   		},
		stride
	);
}

} // namespace detail

template <typename F, std::size_t N>
inline
auto dispatch_inner_loop_strides(
	F &&callable, 
	const joint_layout &layout,
	std::integral_constant<std::size_t, N>
)
{
	return dispatch_inner_loop_strides(
		std::forward<F>(callable), 
		layout,
		std::make_index_sequence<N>()
	);
}

template <typename F, std::size_t... Is>
inline
auto dispatch_inner_loop_strides(
	F &&callable, 
	const joint_layout &layout,
	std::index_sequence<Is...>
)
{
	const auto extents = layout.get_extents();
	if (extents.empty())
	{
		// Zero dimensions, no stride to be obtained. Invoke the callable with
		// a broadcasting tag for every operand.
		REXLIB_CONST_CONSTEXPR broadcasting_stride_tag broadcast;
		return std::forward<F>(callable)(
			std::make_tuple(((void)Is, broadcast)...)
		);
	}

	REXLIB_CONST_CONSTEXPR std::size_t inner_index = 0;
	return detail::dispatch_strides(
		[&callable] (auto... stride_tags)
		{
			return std::forward<F>(callable)(std::make_tuple(stride_tags...));
		},
		(layout.get_strides(Is)[inner_index])...
	);
}

} // namespace cpu
} // namespace rex
