// SPDX-License-Identifier: GPL-3.0-only

#include "sequence_loop.hpp"

#include "inner_loop_stride_dispatch.hpp"
#include "loop_schedule.hpp"
#include "strided_pointer_iterator.hpp"

namespace rex
{
namespace cpu
{
namespace detail
{

template <typename Generator, typename T, typename Stride>
inline
void run_sequence_loop_impl(
	const Generator &generator,
	T *pointer,
	std::size_t begin,
	std::size_t end,
	Stride stride
)
{
	// Displacing the pointer by where the range starts, and counting from
	// there, so that the generator is handed the position within the whole
	// sequence rather than within this range. A sequence is written from its
	// index alone, so that index is the only thing tying a chunk of it back
	// to the operand it belongs to.
	auto ite = make_strided_pointer_iterator(
		pointer + static_cast<std::ptrdiff_t>(begin)*stride,
		stride
	);
	for (std::size_t i = begin; i < end; ++i)
	{
		generator(ite.data(), i);
		++ite;
	}
}

} // namespace detail

template <typename Generator, typename T>
inline
void run_sequence_loop(
	const Generator &generator,
	T *pointer,
	std::size_t count,
	std::ptrdiff_t stride
)
{
	run_sequence_loop(generator, pointer, count, stride, loop_schedule());
}

template <typename Generator, typename T>
inline
void run_sequence_loop(
	const Generator &generator,
	T *pointer,
	std::size_t count,
	std::ptrdiff_t stride,
	const loop_schedule &schedule
)
{
	// The unit stride is resolved to a tag rather than left as a runtime
	// value because it is the case that vectorizes, and it is the case every
	// freshly allocated result takes. Only a caller writing into a view of
	// its own takes the other branch.
	//
	// Resolved outside the split, so that the body handed to the schedule is
	// one type rather than one per branch.
	if (stride == 1)
	{
		schedule.run(
			count,
			[&generator, pointer] (std::size_t begin, std::size_t end)
			{
				detail::run_sequence_loop_impl(
					generator,
					pointer,
					begin,
					end,
					contiguous_stride_tag()
				);
			}
		);
	}
	else
	{
		schedule.run(
			count,
			[&generator, pointer, stride] (std::size_t begin, std::size_t end)
			{
				detail::run_sequence_loop_impl(
					generator,
					pointer,
					begin,
					end,
					stride
				);
			}
		);
	}
}

} // namespace cpu
} // namespace rex
