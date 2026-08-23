// SPDX-License-Identifier: GPL-3.0-only

#include "elementwise_loop.hpp"

#include "inner_loop_stride_dispatch.hpp"
#include "loop_schedule.hpp"
#include "strided_pointer_iterator.hpp"

#include <xmipp4/core/layout/joint_cursor.hpp>
#include <xmipp4/core/platform/assert.hpp>

#include <algorithm>
#include <initializer_list>
#include <tuple>
#include <utility>
#include <type_traits>

namespace rex
{
namespace cpu
{
namespace detail
{

template <typename InnerLoop, typename... Pointers, std::size_t... Is>
inline
void run_elementwise_outer_loop_impl(
	InnerLoop &&inner_loop,
	const joint_layout &layout,
	std::size_t begin,
	std::size_t end,
	const std::tuple<Pointers...> &pointers,
	std::index_sequence<Is...>
)
{
	if (begin >= end)
	{
		return;
	}

	joint_cursor ite;

	// seek() answers what iter() answers, the run left in the innermost axis,
	// so the walk needs to read nothing off the cursor but the offsets. It is
	// also what keeps the rank zero case from needing a shape of its own:
	// a layout of no axes holds one position and reports a run of one, where
	// asking next() for the remainder would answer zero and be
	// indistinguishable from an exhausted iteration.
	auto run = layout.seek(ite, begin);
	if (run == 0)
	{
		return;
	}

	const auto offsets = ite.get_offsets(); // Stable address
	auto remaining = end - begin;
	for (;;)
	{
		// Never more than the run left in the current vector, so that the
		// inner loop is always handed one contiguous stretch of one axis, and
		// never more than the range asks for, so that two walkers splitting
		// that range visit between them exactly what one walker visits alone.
		const auto count = std::min(run, remaining);
		inner_loop(std::get<Is>(pointers) + offsets[Is]..., count);

		remaining -= count;
		if (remaining == 0)
		{
			// Leaving before the last advance, which would only rewind every
			// axis of every operand for nobody to read.
			break;
		}

		run = layout.next(ite, count);
		REX_ASSERT( run > 0 ); // end was past the end of the layout.
	}
}

template <bool...>
struct bool_pack
{
};

template <bool... Values>
using all_true =
	std::is_same<bool_pack<true, Values...>, bool_pack<Values..., true>>;

template <typename... Strides>
struct all_contiguous
{
	using type = typename all_true<
		std::is_same<Strides, contiguous_stride_tag>::value...
	>::type;
};

template <typename Op, typename... Pointers, std::size_t... Is>
inline
void run_contiguous_element_loop(
	const Op &op,
	const std::tuple<Pointers*...> &pointers,
	std::size_t count,
	std::index_sequence<Is...>
)
{
	static_assert(
		sizeof...(Is) == sizeof...(Pointers), 
		"Index count and pointer count must match."
	);

	for (std::size_t i = 0; i < count; ++i)
	{
		op((std::get<Is>(pointers) + i)...);
	}
}

template <
	typename Op,
	typename... Pointers,
	typename... Strides,
	std::size_t... Is
>
inline
void run_generic_element_loop(
	const Op &op,
	const std::tuple<Pointers*...> &pointers,
	const std::tuple<Strides...> &strides,
	std::size_t count,
	std::index_sequence<Is...>
)
{
	static_assert(
		sizeof...(Is) == sizeof...(Pointers), 
		"Index count and pointer count must match,"
	);
	static_assert(
		sizeof...(Is) == sizeof...(Strides), 
		"Index count and stride count must match."
	);

	auto iterators = std::make_tuple(
		make_strided_pointer_iterator(
			std::get<Is>(pointers),
			std::get<Is>(strides)
		)...
	);
	for (std::size_t i = 0; i < count; ++i)
	{
		op(std::get<Is>(iterators).data()...);
		(void) std::initializer_list<int> {
			(++std::get<Is>(iterators), 0)...
		};
	}
}

template <typename Op, typename... Pointers, typename... Strides>
inline
void dispatch_element_loop(
	const Op &op,
	const std::tuple<Pointers*...> &pointers,
	const std::tuple<Strides...> &/*strides*/,
	std::size_t count,
	std::true_type
)
{
	run_contiguous_element_loop(
		op, 
		pointers, 
		count, 
		std::index_sequence_for<Pointers...>()
	);
}

template <typename Op, typename... Pointers, typename... Strides>
inline
void dispatch_element_loop(
	const Op &op,
	const std::tuple<Pointers*...> &pointers,
	const std::tuple<Strides...> &strides,
	std::size_t count,
	std::false_type
)
{
	run_generic_element_loop(
		op, 
		pointers, 
		strides, 
		count, 
		std::index_sequence_for<Pointers...>()
	);
}

template <typename Op, typename... Pointers, typename... Strides>
inline
void run_element_loop(
	const Op &op,
	const std::tuple<Pointers*...> &pointers,
	const std::tuple<Strides...> &strides,
	std::size_t count
)
{
	dispatch_element_loop(
		op,
		pointers,
		strides,
		count,
		typename all_contiguous<Strides...>::type()
	);
}

} // namespace detail

template <typename InnerLoop, typename... Pointers>
inline
void run_elementwise_outer_loop(
	InnerLoop &&inner_loop,
	const joint_layout &layout,
	Pointers... pointers
)
{
	run_elementwise_outer_loop_range(
		std::forward<InnerLoop>(inner_loop),
		layout,
		0,
		layout.compute_element_count(),
		pointers...
	);
}

template <typename InnerLoop, typename... Pointers>
inline
void run_elementwise_outer_loop_range(
	InnerLoop &&inner_loop,
	const joint_layout &layout,
	std::size_t begin,
	std::size_t end,
	Pointers... pointers
)
{
	detail::run_elementwise_outer_loop_impl(
		std::forward<InnerLoop>(inner_loop),
		layout,
		begin,
		end,
		std::make_tuple(pointers...),
		std::make_index_sequence<sizeof...(Pointers)>()
	);
}

template <typename Kernel, typename... Pointers>
inline
void run_elementwise_vector_loop(
	const Kernel &kernel,
	const joint_layout &layout,
	Pointers... pointers
)
{
	run_elementwise_vector_loop(kernel, layout, loop_schedule(), pointers...);
}

template <typename Kernel, typename... Pointers>
inline
void run_elementwise_vector_loop(
	const Kernel &kernel,
	const joint_layout &layout,
	const loop_schedule &schedule,
	Pointers... pointers
)
{
	// The split goes outside the stride dispatch, not inside it. Inside, the
	// body handed to the schedule would be a distinct type per stride
	// combination and would instantiate the whole of the pool's type erasure
	// 3^N times over; outside, it is instantiated once. The cost is resolving
	// the strides once per chunk rather than once per loop, which is a switch
	// over one stride per operand against a chunk of at least one grain.
	schedule.run(
		layout.compute_element_count(),
		[&kernel, &layout, &pointers...] (std::size_t begin, std::size_t end)
		{
			dispatch_inner_loop_strides(
				[begin, end, &kernel, &layout, &pointers...] (auto strides)
				{
					run_elementwise_outer_loop_range(
						[&kernel, &strides]
						(Pointers... vector_pointers, std::size_t count)
						{
							kernel(
								std::make_tuple(vector_pointers...),
								strides,
								count
							);
						},
						layout,
						begin,
						end,
						pointers...
					);
				},
				layout,
				std::integral_constant<std::size_t, sizeof...(Pointers)>()
			);
		}
	);
}

template <typename Op, typename... Pointers>
inline
void run_elementwise_loop(
	const Op &op,
	const joint_layout &layout,
	Pointers... pointers
)
{
	run_elementwise_loop(op, layout, loop_schedule(), pointers...);
}

template <typename Op, typename... Pointers>
inline
void run_elementwise_loop(
	const Op &op,
	const joint_layout &layout,
	const loop_schedule &schedule,
	Pointers... pointers
)
{
	run_elementwise_vector_loop(
		[&op] (const auto &pointers, const auto &strides, std::size_t count)
		{
			detail::run_element_loop(op, pointers, strides, count);
		},
		layout,
		schedule,
		pointers...
	);
}

} // namespace cpu
} // namespace rex
