// SPDX-License-Identifier: GPL-3.0-only

#include "elementwise_loop.hpp"

#include "element_index.hpp"
#include "inner_loop_stride_dispatch.hpp"
#include "loop_schedule.hpp"
#include "strided_pointer_iterator.hpp"

#include <rexlib/core/layout/joint_cursor.hpp>
#include <rexlib/core/platform/assert.hpp>

#include <algorithm>
#include <initializer_list>
#include <tuple>
#include <utility>
#include <type_traits>

namespace rexlib
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
		inner_loop(ite, count, std::get<Is>(pointers) + offsets[Is]...);

		remaining -= count;
		if (remaining == 0)
		{
			// Leaving before the last advance, which would only rewind every
			// axis of every operand for nobody to read.
			break;
		}

		run = layout.next(ite, count);
		REXLIB_ASSERT( run > 0 ); // end was past the end of the layout.
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

template <
	typename Op,
	typename IndexRun,
	typename... Pointers,
	std::size_t... Is
>
inline
void run_contiguous_element_loop(
	const Op &op,
	const std::tuple<Pointers*...> &pointers,
	const IndexRun &index_run,
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
		invoke_with_index(
			op,
			get_kernel_index(advance_index_run(index_run, i)),
			(std::get<Is>(pointers) + i)...
		);
	}
}

template <
	typename Op,
	typename IndexRun,
	typename... Pointers,
	typename... Strides,
	std::size_t... Is
>
inline
void run_generic_element_loop(
	const Op &op,
	const std::tuple<Pointers*...> &pointers,
	const std::tuple<Strides...> &strides,
	const IndexRun &index_run,
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
		invoke_with_index(
			op,
			get_kernel_index(advance_index_run(index_run, i)),
			std::get<Is>(iterators).data()...
		);
		(void) std::initializer_list<int> {
			(++std::get<Is>(iterators), 0)...
		};
	}
}

template <
	typename Op,
	typename IndexRun,
	typename... Pointers,
	typename... Strides
>
inline
void dispatch_element_loop(
	const Op &op,
	const std::tuple<Pointers*...> &pointers,
	const std::tuple<Strides...> &/*strides*/,
	const IndexRun &index_run,
	std::size_t count,
	std::true_type
)
{
	run_contiguous_element_loop(
		op,
		pointers,
		index_run,
		count,
		std::index_sequence_for<Pointers...>()
	);
}

template <
	typename Op,
	typename IndexRun,
	typename... Pointers,
	typename... Strides
>
inline
void dispatch_element_loop(
	const Op &op,
	const std::tuple<Pointers*...> &pointers,
	const std::tuple<Strides...> &strides,
	const IndexRun &index_run,
	std::size_t count,
	std::false_type
)
{
	run_generic_element_loop(
		op,
		pointers,
		strides,
		index_run,
		count,
		std::index_sequence_for<Pointers...>()
	);
}

template <
	typename Op,
	typename IndexRun,
	typename... Pointers,
	typename... Strides
>
inline
void run_element_loop(
	const Op &op,
	const std::tuple<Pointers*...> &pointers,
	const std::tuple<Strides...> &strides,
	const IndexRun &index_run,
	std::size_t count
)
{
	dispatch_element_loop(
		op,
		pointers,
		strides,
		index_run,
		count,
		typename all_contiguous<Strides...>::type()
	);
}

/**
 * @brief Walk one chunk of a layout's iteration space at vector granularity.
 *
 * The kernel is invoked as `kernel(pointers, strides, count, index_run)`,
 * the index run being handed over even when it is @ref no_index_tag.
 */
template <typename Kernel, typename Indexing, typename... Pointers>
inline
void run_indexed_vector_chunk(
	const Kernel &kernel,
	const joint_layout &layout,
	Indexing indexing,
	std::size_t begin,
	std::size_t end,
	Pointers... pointers
)
{
	dispatch_inner_loop_strides(
		[&] (auto strides)
		{
			dispatch_index_run(
				[&] (auto index_run)
				{
					run_elementwise_outer_loop_impl(
						[&] (
							const joint_cursor &cursor,
							std::size_t count,
							Pointers... vector_pointers
						)
						{
							kernel(
								std::make_tuple(vector_pointers...),
								strides,
								count,
								rebase_index_run(
									index_run,
									cursor.get_offsets().data() +
									sizeof...(Pointers)
								)
							);
						},
						layout,
						begin,
						end,
						std::make_tuple(pointers...),
						std::index_sequence_for<Pointers...>()
					);
				},
				layout,
				sizeof...(Pointers),
				indexing
			);
		},
		layout,
		std::integral_constant<std::size_t, sizeof...(Pointers)>()
	);
}

template <typename Kernel, typename Indexing, typename... Pointers>
inline
void run_indexed_vector_loop_impl(
	const Kernel &kernel,
	const joint_layout &layout,
	Indexing indexing,
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
		[&kernel, &layout, indexing, &pointers...]
		(std::size_t begin, std::size_t end)
		{
			run_indexed_vector_chunk(
				kernel,
				layout,
				indexing,
				begin,
				end,
				pointers...
			);
		}
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
		[&inner_loop] (
			const joint_cursor& /*cursor*/,
			std::size_t count,
			Pointers... vector_pointers
		)
		{
			inner_loop(vector_pointers..., count);
		},
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
	run_indexed_elementwise_vector_loop(
		kernel,
		layout,
		no_index_tag(),
		schedule,
		pointers...
	);
}

template <typename Kernel, typename Indexing, typename... Pointers>
inline
void run_indexed_elementwise_vector_loop(
	const Kernel &kernel,
	const joint_layout &layout,
	Indexing indexing,
	Pointers... pointers
)
{
	run_indexed_elementwise_vector_loop(
		kernel,
		layout,
		indexing,
		loop_schedule(),
		pointers...
	);
}

template <typename Kernel, typename Indexing, typename... Pointers>
inline
void run_indexed_elementwise_vector_loop(
	const Kernel &kernel,
	const joint_layout &layout,
	Indexing indexing,
	const loop_schedule &schedule,
	Pointers... pointers
)
{
	detail::run_indexed_vector_loop_impl(
		[&kernel] (
			const auto &vector_pointers,
			const auto &strides,
			std::size_t count,
			const auto &index_run
		)
		{
			detail::invoke_with_index(
				kernel,
				index_run,
				vector_pointers,
				strides,
				count
			);
		},
		layout,
		indexing,
		schedule,
		pointers...
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
	run_indexed_elementwise_loop(
		op,
		layout,
		no_index_tag(),
		schedule,
		pointers...
	);
}

template <typename Op, typename Indexing, typename... Pointers>
inline
void run_indexed_elementwise_loop(
	const Op &op,
	const joint_layout &layout,
	Indexing indexing,
	Pointers... pointers
)
{
	run_indexed_elementwise_loop(
		op,
		layout,
		indexing,
		loop_schedule(),
		pointers...
	);
}

template <typename Op, typename Indexing, typename... Pointers>
inline
void run_indexed_elementwise_loop(
	const Op &op,
	const joint_layout &layout,
	Indexing indexing,
	const loop_schedule &schedule,
	Pointers... pointers
)
{
	detail::run_indexed_vector_loop_impl(
		[&op] (
			const auto &vector_pointers,
			const auto &strides,
			std::size_t count,
			const auto &index_run
		)
		{
			detail::run_element_loop(
				op,
				vector_pointers,
				strides,
				index_run,
				count
			);
		},
		layout,
		indexing,
		schedule,
		pointers...
	);
}

} // namespace cpu
} // namespace rexlib
