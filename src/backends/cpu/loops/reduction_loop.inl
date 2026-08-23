// SPDX-License-Identifier: GPL-3.0-only

#include "reduction_loop.hpp"

#include "inner_loop_stride_dispatch.hpp"
#include "loop_schedule.hpp"
#include "operand_pointers.hpp"
#include "parallel_grain.hpp"
#include "reduction_element_adaptor.hpp"

#include <xmipp4/core/layout/joint_cursor.hpp>
#include <xmipp4/core/platform/assert.hpp>
#include <xmipp4/core/platform/constexpr.hpp>
#include <xmipp4/core/platform/attributes.hpp>

#include <algorithm>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

namespace xmipp4
{
namespace cpu
{
namespace detail
{

/**
 * @brief One buffer per accumulator type, holding what every thread of a fold
 * split arrived at.
 *
 * Arrays rather than vectors: std::vector<bool> packs its elements into bits,
 * and a reduction over booleans is an ordinary thing to ask for. Two threads
 * writing what look like distinct entries would then be writing the same byte.
 */
template <typename Accumulators>
struct accumulator_buffers;

template <typename... Accumulators>
struct accumulator_buffers<type_list<Accumulators...>>
{
	using type = std::tuple<std::unique_ptr<Accumulators[]>...>;
};

/**
 * @brief Give every buffer the same length.
 *
 * Indexed rather than typed: two accumulators of a kernel may well be of the
 * same type, as the position and the value of an extremum are.
 */
template <typename... Accumulators, std::size_t... As>
inline
void allocate_accumulator_buffers(
	std::tuple<std::unique_ptr<Accumulators[]>...> &buffers,
	std::size_t size,
	std::index_sequence<As...>
)
{
	// make_unique for an array type value-initializes, so this keeps the
	// zeroing the explicit new Accumulators[size]() did.
	(void) std::initializer_list<int> {
		(
			std::get<As>(buffers) = std::make_unique<Accumulators[]>(size),
			0
		)...
	};
}

template <typename... Accumulators>
inline
void allocate_accumulator_buffers(
	std::tuple<std::unique_ptr<Accumulators[]>...> &buffers,
	std::size_t size
)
{
	allocate_accumulator_buffers(
		buffers,
		size,
		std::index_sequence_for<Accumulators...>()
	);
}

/**
 * @brief The half-open slice of the reduced space one chunk folds.
 *
 * The first `count % chunk_count` chunks are one element longer than the rest.
 * No slice is ever empty, which matters because a slice is seeded from its own
 * first element.
 */
inline
void split_reduced_range(
	std::size_t count,
	std::size_t chunk_count,
	std::size_t chunk,
	std::size_t &begin,
	std::size_t &end
) noexcept
{
	const auto quotient = count / chunk_count;
	const auto remainder = count % chunk_count;

	begin = chunk*quotient + std::min(chunk, remainder);
	end = begin + quotient;
	if (chunk < remainder)
	{
		++end;
	}
}

/**
 * @brief Stride of an operand along the innermost axis of a layout.
 *
 * A layout of rank zero holds a single position and no stride to read, which
 * is the shape a reduction over every axis leaves behind.
 */
inline
std::ptrdiff_t get_inner_stride(
	const joint_layout &layout,
	std::size_t operand
)
{
	const auto strides = layout.get_strides(operand);
	return strides.empty() ? 0 : strides[0];
}

/**
 * @brief The accumulators of one strip of consecutive outputs.
 *
 * Everything a kernel does to accumulators lives here: seeding them, folding
 * elements into them, merging another thread's into them, and finishing them
 * into outputs. It knows nothing of how either space is traversed; it is
 * handed pointers already placed and told how many elements follow.
 *
 * A strip is at most @ref capacity outputs wide, which is what keeps the
 * accumulators in cache while the elements feeding them stream past.
 */
template <
	typename Kernel,
	typename Outputs,
	typename Inputs,
	typename KeptStrides,
	typename ReducedStrides
>
class reduction_accumulators;

template <
	typename Kernel,
	typename... Outs,
	typename... Ins,
	typename... KeptStrides,
	typename... ReducedStrides
>
class reduction_accumulators<
	Kernel,
	std::tuple<Outs*...>,
	std::tuple<const Ins*...>,
	std::tuple<KeptStrides...>,
	std::tuple<ReducedStrides...>
>
{
public:
	using output_pointers = std::tuple<Outs*...>;
	using input_pointers = std::tuple<const Ins*...>;
	using kept_strides = std::tuple<KeptStrides...>;
	using reduced_strides = std::tuple<ReducedStrides...>;
	using output_stride_array = std::array<std::ptrdiff_t, sizeof...(Outs)>;

	using accumulator_types = typename Kernel::template accumulators<
		type_list<Outs...>,
		type_list<Ins...>
	>::type;

	static_assert(
		type_list_size<accumulator_types>::value > 0,
		"A reduction kernel must declare at least one accumulator."
	);

	static REX_CONST_CONSTEXPR std::size_t capacity =
		reduction_tile_size<accumulator_types>::value;

	using partial_buffers =
		typename accumulator_buffers<accumulator_types>::type;

	reduction_accumulators(
		const Kernel &kernel,
		const kept_strides &kept_inner_strides,
		const reduced_strides &reduced_inner_strides,
		const output_stride_array &output_strides,
		std::size_t reduction_count
	)
		: m_kernel(kernel)
		, m_kept_strides(kept_inner_strides)
		, m_reduced_strides(reduced_inner_strides)
		, m_output_strides(output_strides)
		, m_reduction_count(reduction_count)
	{
	}

	/**
	 * @brief Start every accumulator from the element it is pointed at.
	 *
	 * Seeding from an element, rather than from an identity, is what lets a
	 * fold with no neutral element be expressed the same way as one that has.
	 */
	void seed(
		const input_pointers &inputs,
		std::size_t width,
		std::size_t position
	)
	{
		seed(inputs, width, position, accumulator_indices(), input_indices());
	}

	/**
	 * @brief Fold a run of consecutive elements into every accumulator.
	 *
	 * @param inputs Operand pointers at the first element of the run.
	 * @param width How many outputs the strip covers.
	 * @param count How many elements the run holds.
	 * @param position Where the run starts in the reduced space.
	 */
	void combine(
		const input_pointers &inputs,
		std::size_t width,
		std::size_t count,
		std::size_t position
	)
	{
		combine(
			inputs,
			width,
			count,
			position,
			strip_outermost(),
			accumulator_indices()
		);
	}

	/**
	 * @brief How much of the reduced space to hand over at once.
	 *
	 * A run handed over whole is walked block by block, each block folding
	 * the length of it before the next one starts. That is what keeps a
	 * block's accumulators in registers, and it is also what turns the read
	 * of the input from one stretch of a strip into a walk of a block's
	 * width, which is only cheap while the rows a block re-walks stay in
	 * cache. Bounding the run bounds what one pass spans.
	 *
	 * Unbounded in the other orientation, where a run is folded into a single
	 * accumulator and the longer it is the less the lanes cost to set up.
	 *
	 * @param width How many outputs the strip covers.
	 * @return std::size_t The longest run worth handing over. Never zero.
	 */
	static std::size_t preferred_run_length(std::size_t width) noexcept
	{
		return preferred_run_length(width, strip_outermost());
	}

	/**
	 * @brief Start every accumulator from the kernel's neutral element.
	 *
	 * Only reachable for a reduction over no elements at all, which an
	 * operation without an identity has no answer for.
	 */
	void set_identity(std::size_t width)
	{
		set_identity(width, identity_available());
	}

	/**
	 * @brief Write every accumulator out as the output it answers.
	 */
	void finalize(const output_pointers &outputs, std::size_t width)
	{
		finalize(outputs, width, accumulator_indices(), output_indices());
	}

	/**
	 * @brief Copy this strip's accumulators into a chunk's partials.
	 */
	void save(
		partial_buffers &buffers,
		std::size_t slot,
		std::size_t width
	) const
	{
		save(buffers, slot, width, accumulator_indices());
	}

	/**
	 * @brief Fold every chunk's partials for this strip into it.
	 *
	 * Ascending in the chunk index, never in the order the chunks happened to
	 * finish: merge keeps the earlier of two equally good answers, so the
	 * order it sees is what decides which one that is.
	 *
	 * @param buffers The partials every chunk left.
	 * @param slot Where this strip sits within one chunk's partials.
	 * @param stride How far apart consecutive chunks sit.
	 * @param chunk_count How many chunks left partials.
	 * @param width How many outputs the strip covers.
	 */
	void merge_all(
		const partial_buffers &buffers,
		std::size_t slot,
		std::size_t stride,
		std::size_t chunk_count,
		std::size_t width
	)
	{
		merge_all(
			buffers,
			slot,
			stride,
			chunk_count,
			width,
			accumulator_indices()
		);
	}

private:
	using accumulator_indices = std::make_index_sequence<
		type_list_size<accumulator_types>::value
	>;
	using input_indices = std::index_sequence_for<Ins...>;
	using output_indices = std::index_sequence_for<Outs...>;
	using identity_available =
		typename has_reduction_identity<Kernel, accumulator_types>::type;

	// Whether the strip belongs outside the fold, which it does when the axis
	// being folded is the contiguous one and the surviving axis is not. The
	// first input decides: the others rarely disagree, and none of them is
	// read more often than it.
	using strip_outermost = std::integral_constant<
		bool,
		std::is_same<
			typename std::tuple_element<0, reduced_strides>::type,
			contiguous_stride_tag
		>::value &&
		!std::is_same<
			typename std::tuple_element<0, kept_strides>::type,
			contiguous_stride_tag
		>::value
	>;

	static std::size_t preferred_run_length(
		std::size_t,
		std::true_type
	) noexcept
	{
		return std::numeric_limits<std::size_t>::max();
	}

	static std::size_t preferred_run_length(
		std::size_t width,
		std::false_type
	) noexcept
	{
		REX_CONST_CONSTEXPR std::size_t element_footprint =
			accumulator_footprint<type_list<Ins...>>::value;

		const auto span = std::max<std::size_t>(width*element_footprint, 1);
		return std::max<std::size_t>(
			detail::reduction_strip_pass_budget / span,
			1
		);
	}

	template <std::size_t... As, std::size_t... Is>
	void seed(
		const input_pointers &inputs,
		std::size_t width,
		std::size_t position,
		std::index_sequence<As...>,
		std::index_sequence<Is...>
	)
	{
		for (std::size_t j = 0; j < width; ++j)
		{
			const auto element = step_pointers(
				inputs,
				j,
				m_kept_strides,
				input_indices()
			);

			m_kernel.seed(
				std::get<As>(m_tiles)[j]...,
				std::get<Is>(element)...,
				position
			);
		}
	}

	/**
	 * @brief Fold a run with the strip outside it.
	 *
	 * Taken when the axis being folded is the contiguous one. Each output's
	 * run is then read as the contiguous stream it is, into an accumulator
	 * that stays put for the whole of it, which is what `combine_run` is
	 * shaped for. Keeping the strip inside here would turn that stream into a
	 * gather one strip apart, which costs an order of magnitude.
	 */
	template <std::size_t... As>
	void combine(
		const input_pointers &inputs,
		std::size_t width,
		std::size_t count,
		std::size_t position,
		std::true_type,
		std::index_sequence<As...>
	)
	{
		for (std::size_t j = 0; j < width; ++j)
		{
			const auto column = step_pointers(
				inputs,
				j,
				m_kept_strides,
				input_indices()
			);

			m_kernel.combine_run(
				std::make_tuple(&std::get<As>(m_tiles)[j]...),
				column,
				m_reduced_strides,
				count,
				position
			);
		}
	}

	/**
	 * @brief Fold a run with the strip inside it.
	 *
	 * Taken when the surviving axis is the contiguous one. Consecutive
	 * outputs and the elements feeding them are then both walked
	 * contiguously, which is what `combine_strip` is shaped for: one element
	 * of the reduced space into every accumulator of the strip, the two walks
	 * running side by side.
	 */
	template <std::size_t... As>
	void combine(
		const input_pointers &inputs,
		std::size_t width,
		std::size_t count,
		std::size_t position,
		std::false_type,
		std::index_sequence<As...>
	)
	{
		m_kernel.combine_strip(
			std::make_tuple(std::get<As>(m_tiles).data()...),
			inputs,
			m_kept_strides,
			m_reduced_strides,
			width,
			count,
			position
		);
	}

	void set_identity(std::size_t width, std::true_type)
	{
		set_identity(width, accumulator_indices());
	}

	REX_NORETURN
	void set_identity(std::size_t, std::false_type) const
	{
		throw std::invalid_argument(
			"run_reduction_loop: A reduction over no elements has no answer "
			"for an operation without an identity."
		);
	}

	template <std::size_t... As>
	void set_identity(std::size_t width, std::index_sequence<As...>)
	{
		for (std::size_t j = 0; j < width; ++j)
		{
			m_kernel.identity(std::get<As>(m_tiles)[j]...);
		}
	}

	template <std::size_t... As, std::size_t... Os>
	void finalize(
		const output_pointers &outputs,
		std::size_t width,
		std::index_sequence<As...>,
		std::index_sequence<Os...>
	)
	{
		for (std::size_t j = 0; j < width; ++j)
		{
			const auto element = step_pointers(
				outputs,
				j,
				m_output_strides,
				output_indices()
			);

			m_kernel.finalize(
				std::get<Os>(element)...,
				std::get<As>(m_tiles)[j]...,
				m_reduction_count
			);
		}
	}

	template <std::size_t... As>
	void save(
		partial_buffers &buffers,
		std::size_t slot,
		std::size_t width,
		std::index_sequence<As...>
	) const
	{
		for (std::size_t j = 0; j < width; ++j)
		{
			(void) std::initializer_list<int> {
				(
					std::get<As>(buffers)[slot + j] =
						std::get<As>(m_tiles)[j],
					0
				)...
			};
		}
	}

	template <std::size_t... As>
	void merge_all(
		const partial_buffers &buffers,
		std::size_t slot,
		std::size_t stride,
		std::size_t chunk_count,
		std::size_t width,
		std::index_sequence<As...>
	)
	{
		for (std::size_t j = 0; j < width; ++j)
		{
			// The first chunk is the answer so far, for the reason a fold is
			// seeded from an element rather than from an identity.
			(void) std::initializer_list<int> {
				(
					std::get<As>(m_tiles)[j] =
						std::get<As>(buffers)[slot + j],
					0
				)...
			};

			for (std::size_t c = 1; c < chunk_count; ++c)
			{
				const auto other = c*stride + slot + j;
				m_kernel.merge(
					std::get<As>(m_tiles)[j]...,
					std::get<As>(buffers)[other]...
				);
			}
		}
	}

	using tiles_type =
		typename accumulator_tiles<accumulator_types, capacity>::type;

	const Kernel &m_kernel;
	kept_strides m_kept_strides;
	reduced_strides m_reduced_strides;
	output_stride_array m_output_strides;
	std::size_t m_reduction_count;
	tiles_type m_tiles;
};


/**
 * @brief Drives a reduction over a planned iteration.
 *
 * Owns the traversal of both spaces and delegates every arithmetic decision to
 * @ref reduction_accumulators. The surviving space is walked strip by strip;
 * for each strip the reduced space is walked run by run and folded in.
 *
 * The traversal is a class so that the state each level needs, the layouts,
 * the cursors and the base pointers, is named once instead of being threaded
 * through argument lists that grow with every level.
 */
template <
	typename Kernel,
	typename Outputs,
	typename Inputs,
	typename KeptStrides,
	typename ReducedStrides
>
class reduction_loop_runner;

template <
	typename Kernel,
	typename... Outs,
	typename... Ins,
	typename... KeptStrides,
	typename... ReducedStrides
>
class reduction_loop_runner<
	Kernel,
	std::tuple<Outs*...>,
	std::tuple<const Ins*...>,
	std::tuple<KeptStrides...>,
	std::tuple<ReducedStrides...>
>
{
public:
	using output_pointers = std::tuple<Outs*...>;
	using input_pointers = std::tuple<const Ins*...>;
	using kept_strides = std::tuple<KeptStrides...>;
	using reduced_strides = std::tuple<ReducedStrides...>;

	using accumulators_type = reduction_accumulators<
		Kernel,
		output_pointers,
		input_pointers,
		kept_strides,
		reduced_strides
	>;
	using partial_buffers = typename accumulators_type::partial_buffers;

	reduction_loop_runner(
		const Kernel &kernel,
		const joint_layout &kept_layout,
		const joint_layout &reduced_layout,
		std::size_t reduction_count,
		const output_pointers &outputs,
		const input_pointers &inputs,
		const kept_strides &kept_inner_strides,
		const reduced_strides &reduced_inner_strides
	)
		: m_kept_layout(kept_layout)
		, m_reduced_layout(reduced_layout)
		, m_reduction_count(reduction_count)
		, m_outputs(outputs)
		, m_inputs(inputs)
		, m_kept_strides(kept_inner_strides)
		, m_reduced_strides(reduced_inner_strides)
		, m_output_strides(read_output_strides(kept_layout))
		, m_accumulators(
			kernel,
			kept_inner_strides,
			reduced_inner_strides,
			m_output_strides,
			reduction_count
		)
		, m_reduced_run(0)
	{
	}

	/**
	 * @brief Reduce into a range of the output elements.
	 *
	 * The range is over the surviving space, so it names a set of outputs.
	 * Two runners covering a range between them fill exactly the outputs one
	 * runner over the whole of it fills, each folded by one of them and in the
	 * order it would be folded serially. That is what lets the surviving space
	 * be shared out with no merging.
	 */
	void reduce_range(std::size_t begin, std::size_t end)
	{
		arm_reduced_space(0);

		for_each_strip(
			begin,
			end,
			[this] (
				const input_pointers &inputs,
				const output_pointers &outputs,
				std::size_t /*position*/,
				std::size_t width
			)
			{
				fold_whole_space(inputs, width);
				m_accumulators.finalize(outputs, width);
			}
		);
	}

	/**
	 * @brief Fold one slice of the reduced space into a chunk's partials.
	 *
	 * Walks the whole surviving space, folds only `[begin, end)` of the
	 * reduced space into each strip, and leaves the accumulators in @p buffers
	 * for @ref merge_partials to finish.
	 *
	 * The accumulators are built in this runner's own strip and copied out
	 * once, rather than folded into @p buffers directly: this path is only
	 * taken when the surviving space is small, so several chunks' entries
	 * share a cache line and folding into them would be false sharing on the
	 * hottest variable of the loop.
	 */
	void fold_slice_into(
		std::size_t chunk,
		std::size_t kept_count,
		std::size_t begin,
		std::size_t end,
		partial_buffers &buffers
	)
	{
		REX_ASSERT( begin < end );
		arm_reduced_space(begin);

		const auto base = chunk*kept_count;
		for_each_strip(
			0,
			kept_count,
			[this, base, begin, end, &buffers] (
				const input_pointers &inputs,
				const output_pointers &/*outputs*/,
				std::size_t position,
				std::size_t width
			)
			{
				fold_slice(inputs, width, begin, end);
				m_accumulators.save(buffers, base + position, width);
			}
		);
	}

	/**
	 * @brief Merge every chunk's partials and write the outputs.
	 */
	void merge_partials(
		std::size_t chunk_count,
		std::size_t kept_count,
		const partial_buffers &buffers
	)
	{
		for_each_strip(
			0,
			kept_count,
			[this, chunk_count, kept_count, &buffers] (
				const input_pointers &/*inputs*/,
				const output_pointers &outputs,
				std::size_t position,
				std::size_t width
			)
			{
				m_accumulators.merge_all(
					buffers,
					position,
					kept_count,
					chunk_count,
					width
				);
				m_accumulators.finalize(outputs, width);
			}
		);
	}

private:
	static REX_CONST_CONSTEXPR std::size_t input_count = sizeof...(Ins);
	static REX_CONST_CONSTEXPR std::size_t strip_width =
		accumulators_type::capacity;

	using input_indices = std::index_sequence_for<Ins...>;
	using output_indices = std::index_sequence_for<Outs...>;
	using output_stride_array =
		typename accumulators_type::output_stride_array;

	/**
	 * @brief The outputs are operands of the kept layout, after the inputs.
	 */
	static output_stride_array read_output_strides(
		const joint_layout &kept_layout
	)
	{
		output_stride_array result;
		for (std::size_t i = 0; i < result.size(); ++i)
		{
			result[i] = get_inner_stride(kept_layout, input_count + i);
		}
		return result;
	}

	/**
	 * @brief Position the reduced cursor where a fold is to start.
	 *
	 * Every strip folds the same stretch of the reduced space, so the cursor
	 * is placed once and copied back per strip. Seeking per strip would
	 * allocate per strip, seek() building a fresh cursor every call.
	 */
	void arm_reduced_space(std::size_t begin)
	{
		m_reduced_run = m_reduced_layout.seek(m_reduced_cursor, begin);
		m_reduced_origin = m_reduced_cursor;
	}

	/**
	 * @brief Walk a range of the surviving space, strip by strip.
	 *
	 * @param begin,end The range of the surviving space to walk.
	 * @param action Invoked per strip with the operand pointers displaced to
	 * its start, its position in the surviving space, and its width.
	 */
	template <typename Action>
	void for_each_strip(
		std::size_t begin,
		std::size_t end,
		const Action &action
	)
	{
		if (begin >= end)
		{
			return;
		}

		auto run = m_kept_layout.seek(m_kept_cursor, begin);
		if (run == 0)
		{
			return; // The outputs are empty, so there is nothing to fill.
		}

		auto position = begin;
		auto remaining = end - begin;
		for (;;)
		{
			// The cursor may be parked inside a vector, in which case what is
			// left of it is what gets walked.
			const auto count = std::min(run, remaining);
			for_each_strip_of_vector(position, count, action);

			position += count;
			remaining -= count;
			if (remaining == 0)
			{
				break;
			}

			run = m_kept_layout.next(m_kept_cursor, count);
			REX_ASSERT( run > 0 );
		}
	}

	template <typename Action>
	void for_each_strip_of_vector(
		std::size_t position,
		std::size_t vector_size,
		const Action &action
	)
	{
		const auto offsets = m_kept_cursor.get_offsets();
		const auto inputs = offset_pointers(
			m_inputs,
			offsets.data(),
			input_indices()
		);
		const auto outputs = offset_pointers(
			m_outputs,
			offsets.data() + input_count,
			output_indices()
		);

		for (std::size_t begin = 0; begin < vector_size; begin += strip_width)
		{
			const auto width = std::min(strip_width, vector_size - begin);

			action(
				step_pointers(inputs, begin, m_kept_strides, input_indices()),
				step_pointers(
					outputs,
					begin,
					m_output_strides,
					output_indices()
				),
				position + begin,
				width
			);
		}
	}

	/**
	 * @brief Fold the whole reduced space into the current strip.
	 */
	void fold_whole_space(const input_pointers &inputs, std::size_t width)
	{
		if (m_reduced_run == 0)
		{
			m_accumulators.set_identity(width);
			return;
		}

		fold_slice(inputs, width, 0, m_reduction_count);
	}

	/**
	 * @brief Fold `[begin, end)` of the reduced space into the current strip.
	 *
	 * The positions handed to the kernel are absolute within the reduced
	 * space, never relative to the slice. An operation reporting where it
	 * found something depends on it, and so does the merge that follows: the
	 * earlier of two equally good answers is only the earlier one if both were
	 * numbered from the same origin.
	 */
	void fold_slice(
		const input_pointers &inputs,
		std::size_t width,
		std::size_t begin,
		std::size_t end
	)
	{
		REX_ASSERT( begin < end );

		m_reduced_cursor = m_reduced_origin;

		auto run = m_reduced_run;
		auto remaining = end - begin;
		auto position = begin;

		// The first element seeds the accumulators, the rest of its run is
		// folded in behind it.
		const auto first = current_reduced_pointers(inputs);
		m_accumulators.seed(first, width, position);

		auto count = std::min(run, remaining) - 1;
		fold_run(
			step_pointers(first, 1, m_reduced_strides, input_indices()),
			width,
			count,
			position + 1
		);

		++count;
		position += count;
		remaining -= count;

		while (remaining)
		{
			run = m_reduced_layout.next(m_reduced_cursor, count);
			REX_ASSERT( run > 0 );

			count = std::min(run, remaining);
			fold_run(
				current_reduced_pointers(inputs),
				width,
				count,
				position
			);

			position += count;
			remaining -= count;
		}
	}

	/**
	 * @brief Fold one run of the reduced space into the current strip.
	 *
	 * Handed over in passes no longer than the accumulators want to see at
	 * once, so that whatever they do with a run is bounded in what it spans.
	 * One pass for the whole run is the usual answer.
	 */
	void fold_run(
		const input_pointers &base,
		std::size_t width,
		std::size_t count,
		std::size_t position
	)
	{
		const auto limit = accumulators_type::preferred_run_length(width);

		std::size_t done = 0;
		while (done < count)
		{
			const auto pass = std::min(limit, count - done);
			m_accumulators.combine(
				step_pointers(base, done, m_reduced_strides, input_indices()),
				width,
				pass,
				position + done
			);
			done += pass;
		}
	}

	input_pointers current_reduced_pointers(
		const input_pointers &inputs
	) const
	{
		const auto offsets = m_reduced_cursor.get_offsets();
		return offset_pointers(inputs, offsets.data(), input_indices());
	}

	const joint_layout &m_kept_layout;
	const joint_layout &m_reduced_layout;
	std::size_t m_reduction_count;
	output_pointers m_outputs;
	input_pointers m_inputs;
	kept_strides m_kept_strides;
	reduced_strides m_reduced_strides;
	output_stride_array m_output_strides;
	accumulators_type m_accumulators;
	joint_cursor m_kept_cursor;
	joint_cursor m_reduced_cursor;
	joint_cursor m_reduced_origin;
	std::size_t m_reduced_run;
};

} // namespace detail

namespace detail
{

/**
 * @brief Resolve both layouts' inner strides to tags and hand them over.
 *
 * Only the inputs take part: the outputs are walked through strides read once
 * into an array, so they cost nothing here. The two dispatches together
 * instantiate @p callable up to `3^(2N)` times, where N is the input count.
 */
template <typename Callable, std::size_t... Is>
inline
void dispatch_reduction_strides(
	const joint_layout &kept_layout,
	const joint_layout &reduced_layout,
	std::index_sequence<Is...> input_indices,
	const Callable &callable
)
{
	dispatch_inner_loop_strides(
		[&] (auto kept_strides)
		{
			dispatch_inner_loop_strides(
				[&] (auto reduced_strides)
				{
					callable(kept_strides, reduced_strides);
				},
				reduced_layout,
				input_indices
			);
		},
		kept_layout,
		input_indices
	);
}

/**
 * @brief Build a runner for a set of operands and resolved strides.
 *
 * Spares every call site the runner's five template arguments, which are
 * deducible from what it is handed.
 */
template <
	typename Kernel,
	typename Outputs,
	typename Inputs,
	typename KeptStrides,
	typename ReducedStrides
>
inline
reduction_loop_runner<Kernel, Outputs, Inputs, KeptStrides, ReducedStrides>
make_reduction_runner(
	const Kernel &kernel,
	const joint_layout &kept_layout,
	const joint_layout &reduced_layout,
	std::size_t reduction_count,
	const Outputs &outputs,
	const Inputs &inputs,
	const KeptStrides &kept_strides,
	const ReducedStrides &reduced_strides
)
{
	return {
		kernel,
		kept_layout,
		reduced_layout,
		reduction_count,
		outputs,
		inputs,
		kept_strides,
		reduced_strides
	};
}

/**
 * @brief Whether to share out the space a reduction folds over, rather than
 * the space that survives it.
 *
 * The surviving space is preferred wherever it has the parallelism to spare:
 * splitting it needs no merge, no buffer, and reassociates nothing, so its
 * answer is what one thread computes down to the last bit. Twice the worker
 * count rather than once, so that a ragged finish has somewhere to go.
 *
 * Only when there are too few outputs for that is the fold itself worth
 * splitting, of which a reduction to a scalar is the extreme: one output, and
 * all of the work behind it. Below one grain nothing is worth splitting, and
 * the schedule refuses on its own.
 */
inline
bool should_split_reduced_space(
	std::size_t kept_count,
	std::size_t reduction_count,
	std::size_t workers
) noexcept
{
	return
		workers > 1 &&
		kept_count > 0 &&
		kept_count < 2*workers &&
		reduction_count >= 2*workers &&
		kept_count*reduction_count >= get_parallel_grain_size();
}

/**
 * @brief Fold a reduction by splitting the space it folds over.
 *
 * Each chunk folds its own slice of the reduced space over the whole surviving
 * space into partials of its own, and the thread that joins them merges those
 * and finalizes once. The kernel's merge is what makes this expressible, and
 * this is the only caller of it.
 *
 * @warning Splitting the fold reassociates it, so a sum of floating point
 * values stops being bit for bit what one thread computes, and stops being the
 * same across worker counts.
 */
template <
	typename Kernel,
	typename Outputs,
	typename Inputs,
	typename KeptStrides,
	typename ReducedStrides
>
inline
void run_reduction_fold_split(
	const Kernel &kernel,
	const joint_layout &kept_layout,
	const joint_layout &reduced_layout,
	std::size_t reduction_count,
	const Outputs &outputs,
	const Inputs &inputs,
	const KeptStrides &kept_strides,
	const ReducedStrides &reduced_strides,
	std::size_t kept_count,
	std::size_t chunk_count,
	const loop_schedule &schedule
)
{
	const auto make_runner = [&] ()
	{
		return make_reduction_runner(
			kernel,
			kept_layout,
			reduced_layout,
			reduction_count,
			outputs,
			inputs,
			kept_strides,
			reduced_strides
		);
	};

	typename decltype(make_runner())::partial_buffers buffers;
	allocate_accumulator_buffers(buffers, chunk_count*kept_count);

	// Cut here rather than letting the pool cut, so that a chunk index names
	// the same slice however many chunks the pool runs at once, and so that
	// this still works when it declines to split at all.
	schedule.with_grain(1).run(
		chunk_count,
		[&] (std::size_t first, std::size_t last)
		{
			for (std::size_t chunk = first; chunk < last; ++chunk)
			{
				std::size_t begin;
				std::size_t end;
				split_reduced_range(
					reduction_count,
					chunk_count,
					chunk,
					begin,
					end
				);

				make_runner().fold_slice_into(
					chunk,
					kept_count,
					begin,
					end,
					buffers
				);
			}
		}
	);

	make_runner().merge_partials(chunk_count, kept_count, buffers);
}

/**
 * @brief Fold a reduction by splitting the space that survives it.
 *
 * Each chunk fills outputs of its own, folded start to finish in the order one
 * thread would fold them, so nothing has to be merged and the answer is
 * unchanged down to the last bit.
 */
template <typename Kernel, typename Outputs, typename Inputs, std::size_t... Is>
inline
void run_reduction_output_split(
	const Kernel &kernel,
	const joint_layout &kept_layout,
	const joint_layout &reduced_layout,
	std::size_t reduction_count,
	const Outputs &outputs,
	const Inputs &inputs,
	std::size_t kept_count,
	const loop_schedule &schedule,
	std::index_sequence<Is...> input_indices
)
{
	// One iteration of the surviving space folds the whole reduced space, so
	// that is what an iteration costs and what the grain is stated in.
	const auto cost = std::max<std::size_t>(reduction_count, 1);

	// The split is outside the stride dispatch. Inside, the body handed to the
	// pool would be a distinct type per pair of stride combinations and would
	// instantiate the type erasure 3^(2N) times over.
	schedule.with_grain(grain_for_cost(cost)).run(
		kept_count,
		[&] (std::size_t begin, std::size_t end)
		{
			dispatch_reduction_strides(
				kept_layout,
				reduced_layout,
				input_indices,
				[&] (auto kept_strides, auto reduced_strides)
				{
					// A runner of its own per chunk, so that the cursors and
					// the accumulators it carries live on the stack of
					// whichever thread runs it.
					make_reduction_runner(
						kernel,
						kept_layout,
						reduced_layout,
						reduction_count,
						outputs,
						inputs,
						kept_strides,
						reduced_strides
					).reduce_range(begin, end);
				}
			);
		}
	);
}

} // namespace detail

template <typename Kernel, typename... Outs, typename... Ins>
inline
void run_reduction_vector_loop(
	const Kernel &kernel,
	const joint_layout &kept_layout,
	const joint_layout &reduced_layout,
	std::size_t reduction_count,
	const std::tuple<Outs*...> &outputs,
	const std::tuple<const Ins*...> &inputs
)
{
	run_reduction_vector_loop(
		kernel,
		kept_layout,
		reduced_layout,
		reduction_count,
		outputs,
		inputs,
		loop_schedule()
	);
}

template <typename Kernel, typename... Outs, typename... Ins>
inline
void run_reduction_vector_loop(
	const Kernel &kernel,
	const joint_layout &kept_layout,
	const joint_layout &reduced_layout,
	std::size_t reduction_count,
	const std::tuple<Outs*...> &outputs,
	const std::tuple<const Ins*...> &inputs,
	const loop_schedule &schedule
)
{
	REX_CONST_CONSTEXPR std::index_sequence_for<Ins...> input_indices {};

	const auto kept_count = kept_layout.compute_element_count();
	const auto workers = schedule.get_concurrency();

	if (!detail::should_split_reduced_space(
			kept_count,
			reduction_count,
			workers
		))
	{
		detail::run_reduction_output_split(
			kernel,
			kept_layout,
			reduced_layout,
			reduction_count,
			outputs,
			inputs,
			kept_count,
			schedule,
			input_indices
		);

		return;
	}

	const auto chunk_count = std::min(reduction_count, workers);
	detail::dispatch_reduction_strides(
		kept_layout,
		reduced_layout,
		input_indices,
		[&] (auto kept_strides, auto reduced_strides)
		{
			detail::run_reduction_fold_split(
				kernel,
				kept_layout,
				reduced_layout,
				reduction_count,
				outputs,
				inputs,
				kept_strides,
				reduced_strides,
				kept_count,
				chunk_count,
				schedule
			);
		}
	);
}

template <typename Kernel, typename... Outs, typename... Ins>
inline
void run_reduction_loop(
	const Kernel &kernel,
	const joint_layout &kept_layout,
	const joint_layout &reduced_layout,
	std::size_t reduction_count,
	const std::tuple<Outs*...> &outputs,
	const std::tuple<const Ins*...> &inputs
)
{
	run_reduction_loop(
		kernel,
		kept_layout,
		reduced_layout,
		reduction_count,
		outputs,
		inputs,
		loop_schedule()
	);
}

template <typename Kernel, typename... Outs, typename... Ins>
inline
void run_reduction_loop(
	const Kernel &kernel,
	const joint_layout &kept_layout,
	const joint_layout &reduced_layout,
	std::size_t reduction_count,
	const std::tuple<Outs*...> &outputs,
	const std::tuple<const Ins*...> &inputs,
	const loop_schedule &schedule
)
{
	run_reduction_vector_loop(
		make_reduction_element_adaptor(kernel),
		kept_layout,
		reduced_layout,
		reduction_count,
		outputs,
		inputs,
		schedule
	);
}

} // namespace cpu
} // namespace xmipp4
