// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "reduction_kernel_traits.hpp"

#include <xmipp4/core/platform/cpp_attributes.hpp>

#include <cstddef>
#include <tuple>
#include <utility>

namespace xmipp4
{
namespace cpu
{

/**
 * @brief Presents an element reduction kernel as a vector one.
 *
 * @ref run_reduction_vector_loop hands its kernel a whole run or a whole
 * strip at a time; an element kernel answers about one element at a time.
 * This adapts the second to the first by supplying the two bulk members,
 * `combine_run` and `combine_strip`, as loops over the kernel's `combine`,
 * and forwarding everything else untouched. It is what
 * @ref run_reduction_loop is written in terms of, and it is the reason an
 * operation that has nothing to gain from writing its own vector kernel does
 * not have to.
 *
 * Only the bulk fold is adapted. Seeding, merging, finalizing and the
 * identity stay at element granularity, because each of them happens once per
 * output rather than once per element folded, and vectorizing them would buy
 * a fraction of a fraction.
 *
 * `combine_run` folds one run into one accumulator, which is a chain of
 * dependent operations: the next fold cannot start until the last has
 * finished, however wide the vector registers are and whatever the
 * instruction set. That is the shape of a reduction along the contiguous
 * axis, and it is where a reduction leaves the most on the table.
 *
 * When the kernel says its fold may be reassociated, the run is instead dealt
 * out over @ref reduction_fold_lane_count sub-accumulators, consecutive
 * elements going to consecutive lanes, and they are merged once the run is
 * over. The lanes are independent, so the chain is broken as many ways as
 * there are lanes, and consecutive elements landing in consecutive lanes is
 * what keeps the reads one contiguous stretch. A run too short to deal out is
 * folded serially instead.
 *
 * A kernel that does not say so is folded serially, which is the answer a
 * single accumulator arrives at, element by element, in order. See
 * @ref has_reassociable_fold for what saying so commits to.
 *
 * @tparam Kernel The element reduction kernel to adapt. Held by value, as
 * every other holder of one does: a reduction kernel is an empty object
 * carrying a rule rather than any state, so owning it costs nothing and
 * spares this class a lifetime its callers would have to honour.
 *
 * @see run_reduction_loop
 * @see run_reduction_vector_loop
 * @see has_reassociable_fold
 */
template <typename Kernel>
class reduction_element_adaptor
{
public:
	/**
	 * @brief Wrap an element kernel.
	 *
	 * @param kernel The kernel to adapt.
	 */
	explicit reduction_element_adaptor(Kernel kernel);

	/**
	 * @brief The accumulators the wrapped kernel declares.
	 */
	template <typename Outputs, typename Inputs>
	struct accumulators
		: Kernel::template accumulators<Outputs, Inputs>
	{
	};

	/**
	 * @brief Start every accumulator from the element it is pointed at.
	 *
	 * Forwarded unchanged: seeding happens once per output, not once per
	 * element.
	 */
	template <typename... Args>
	void seed(Args &&...args) const;

	/**
	 * @brief Fold a run of consecutive elements into one accumulator set.
	 *
	 * @tparam Accumulators Accumulator types, one per accumulator.
	 * @tparam Ins Input element types, one per input.
	 * @tparam Strides Resolved inner strides of the reduced layout.
	 * @param accumulators Pointer to each accumulator being folded into.
	 * @param inputs Pointer to each input at the first element of the run.
	 * @param strides How far one step along the run moves each input.
	 * @param count How many elements the run holds.
	 * @param position Where the run starts in the reduced space.
	 */
	template <typename... Accumulators, typename... Ins, typename... Strides>
	void combine_run(
		const std::tuple<Accumulators*...> &accumulators,
		const std::tuple<const Ins*...> &inputs,
		const std::tuple<Strides...> &strides,
		std::size_t count,
		std::size_t position
	) const;

	/**
	 * @brief Fold a run of the reduced space into every accumulator of a
	 * strip.
	 *
	 * @tparam Accumulators Accumulator types, one per accumulator.
	 * @tparam Ins Input element types, one per input.
	 * @tparam KeptStrides Resolved inner strides of the kept layout.
	 * @tparam ReducedStrides Resolved inner strides of the reduced layout.
	 * @param accumulators Pointer to the first accumulator of each strip.
	 * @param inputs Pointer to each input at the first element of the run.
	 * @param kept_strides How far one step along the strip moves each input.
	 * @param reduced_strides How far one step along the run moves each input.
	 * @param width How many outputs the strip covers.
	 * @param count How many elements of the reduced space the run holds.
	 * @param position Where the run starts in the reduced space.
	 */
	template <
		typename... Accumulators,
		typename... Ins,
		typename... KeptStrides,
		typename... ReducedStrides
	>
	void combine_strip(
		const std::tuple<Accumulators*...> &accumulators,
		const std::tuple<const Ins*...> &inputs,
		const std::tuple<KeptStrides...> &kept_strides,
		const std::tuple<ReducedStrides...> &reduced_strides,
		std::size_t width,
		std::size_t count,
		std::size_t position
	) const;

	/**
	 * @brief Fold one set of accumulators into another.
	 */
	template <typename... Args>
	void merge(Args &&...args) const;

	/**
	 * @brief Write every accumulator out as the output it answers.
	 */
	template <typename... Args>
	void finalize(Args &&...args) const;

	/**
	 * @brief Start every accumulator from the kernel's neutral element.
	 *
	 * Present only when the wrapped kernel has one, so that
	 * @ref has_reduction_identity answers the same of the adaptor as it does
	 * of what it adapts.
	 */
	template <typename... Accumulators, typename K = Kernel>
	auto identity(Accumulators &...accumulators) const
		-> decltype(std::declval<const K&>().identity(accumulators...));

private:
	using lane_folding = typename has_reassociable_fold<Kernel>::type;

	XMIPP4_NO_UNIQUE_ADDRESS Kernel m_kernel;

	template <
		typename... Accumulators,
		typename... Ins,
		typename... Strides,
		std::size_t... As,
		std::size_t... Is
	>
	void combine_run(
		const std::tuple<Accumulators*...> &accumulators,
		const std::tuple<const Ins*...> &inputs,
		const std::tuple<Strides...> &strides,
		std::size_t count,
		std::size_t position,
		std::false_type,
		std::index_sequence<As...>,
		std::index_sequence<Is...>
	) const;

	template <
		typename... Accumulators,
		typename... Ins,
		typename... Strides,
		std::size_t... As,
		std::size_t... Is
	>
	void combine_run(
		const std::tuple<Accumulators*...> &accumulators,
		const std::tuple<const Ins*...> &inputs,
		const std::tuple<Strides...> &strides,
		std::size_t count,
		std::size_t position,
		std::true_type,
		std::index_sequence<As...>,
		std::index_sequence<Is...>
	) const;

	// One block of the strip, folded over the whole run before the next one
	// starts. Block is a compile time constant, so the loop over it unrolls
	// and its accumulators are values rather than places in the tile.
	template <
		std::size_t Block,
		typename... Accumulators,
		typename... Ins,
		typename... KeptStrides,
		typename... ReducedStrides,
		std::size_t... As,
		std::size_t... Is
	>
	void fold_strip_block(
		const std::tuple<Accumulators*...> &accumulators,
		const std::tuple<const Ins*...> &inputs,
		const std::tuple<KeptStrides...> &kept_strides,
		const std::tuple<ReducedStrides...> &reduced_strides,
		std::size_t first,
		std::size_t count,
		std::size_t position,
		std::integral_constant<std::size_t, Block>,
		std::index_sequence<As...>,
		std::index_sequence<Is...>
	) const;

	// What is left of a strip once the blocks have had it, folded through the
	// tile as before. Never wider than a block.
	template <
		typename... Accumulators,
		typename... Ins,
		typename... KeptStrides,
		typename... ReducedStrides,
		std::size_t... As,
		std::size_t... Is
	>
	void fold_strip_columns(
		const std::tuple<Accumulators*...> &accumulators,
		const std::tuple<const Ins*...> &inputs,
		const std::tuple<KeptStrides...> &kept_strides,
		const std::tuple<ReducedStrides...> &reduced_strides,
		std::size_t first,
		std::size_t width,
		std::size_t count,
		std::size_t position,
		std::index_sequence<As...>,
		std::index_sequence<Is...>
	) const;
};

/**
 * @brief Wrap an element reduction kernel, deducing its type.
 *
 * @tparam Kernel The element reduction kernel.
 * @param kernel The kernel to adapt.
 * @return reduction_element_adaptor<Kernel> The adapted kernel.
 */
template <typename Kernel>
reduction_element_adaptor<Kernel>
make_reduction_element_adaptor(Kernel kernel);

} // namespace cpu
} // namespace xmipp4

#include "reduction_element_adaptor.inl"
