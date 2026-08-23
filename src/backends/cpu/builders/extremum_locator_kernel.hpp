// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/meta/type_list.hpp>
#include <xmipp4/core/platform/cpp_attributes.hpp>

#include <backends/cpu/builders/reduction_compute_type.hpp>
#include <backends/cpu/load_store.hpp>

#include <cstddef>
#include <cstdint>

namespace rex
{
namespace cpu
{

// This header holds scaffolding rather than arithmetic, for the same reason
// fold_reduction_kernel does: it adapts an ordering to the reduction kernel
// concept. The ordering itself belongs in its operation's translation unit,
// or in kernels/ when several operations share it.

/**
 * @brief Reduction kernel locating where an extremum sits.
 *
 * Covers the operations answering with the place of the largest or smallest
 * element rather than with the element itself. What separates one from
 * another is only which of two elements wins, so that is all @p Order says,
 * the way @ref fold_reduction_kernel takes only the binary fold.
 *
 * Two accumulators rather than one pair: the running extremum and the place
 * it was found are kept in separate tiles, so the comparison across the
 * outputs of a tile stays a contiguous walk of each rather than a stride
 * over interleaved fields.
 *
 * The extremum accumulates in the computation type of the operand, while the
 * answer is the index the operation's rule fixes, so neither accumulator has
 * the type of the output. Only the index is written out; the value it was
 * found at is dropped.
 *
 * There is no identity. An extremum of nothing does not exist, and the loop
 * rejects that case rather than inventing one, as NumPy does.
 *
 * @tparam Order Whether a candidate should displace the element held,
 * invoked as `order(candidate, best)`. Must be default constructible, and
 * strict, so that a tie keeps the first place it was seen.
 */
template <typename Order>
class extremum_locator_kernel
{
public:
	/**
	 * @brief The running extremum and the place it was found.
	 *
	 * The extremum is typed on the input rather than on the output, the
	 * output holding an index instead of a value.
	 *
	 * @tparam Outputs type_list of the output element types.
	 * @tparam Inputs type_list of the input element types.
	 */
	template <typename Outputs, typename Inputs>
	struct accumulators
	{
		using type = type_list<
			typename reduction_compute_type<
				typename type_list_element<0, Inputs>::type
			>::type,
			std::int64_t
		>;
	};

	template <typename Accumulator, typename T>
	void seed(
		Accumulator &best,
		std::int64_t &where,
		const T *value,
		std::size_t position
	) const noexcept
	{
		best = load(value);
		where = static_cast<std::int64_t>(position);
	}

	template <typename Accumulator, typename T>
	void combine(
		Accumulator &best,
		std::int64_t &where,
		const T *value,
		std::size_t position
	) const noexcept
	{
		const auto current = static_cast<Accumulator>(load(value));
		if (m_order(current, best))
		{
			best = current;
			where = static_cast<std::int64_t>(position);
		}
	}

	template <typename Accumulator>
	void merge(
		Accumulator &best,
		std::int64_t &where,
		const Accumulator &other_best,
		const std::int64_t &other_where
	) const noexcept
	{
		if (m_order(other_best, best))
		{
			best = other_best;
			where = other_where;
		}
	}

	template <typename U, typename Accumulator>
	void finalize(
		U *result,
		const Accumulator& /*best*/,
		const std::int64_t &where,
		std::size_t /*count*/
	) const noexcept
	{
		store(result, where);
	}

private:
	REX_NO_UNIQUE_ADDRESS Order m_order;
};

} // namespace cpu
} // namespace rex
