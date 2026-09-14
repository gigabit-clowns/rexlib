// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/platform/cpp_attributes.hpp>

#include <backends/cpu/builders/reduction_compute_type.hpp>
#include <backends/cpu/load_store.hpp>

#include <cstddef>
#include <cstdint>

namespace rexlib
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
 * Two elements neither of which displaces the other are a tie, and a tie goes
 * to the smaller linear index. The answer therefore does not depend on the
 * order the elements are visited in, which the reduced layout chooses for
 * locality, nor on the order partial answers are merged in.
 *
 * There is no identity. An extremum of nothing does not exist, and the loop
 * rejects that case rather than inventing one, as NumPy does.
 *
 * @tparam Order Whether a candidate should displace the element held,
 * invoked as `order(candidate, best)`. Must be default constructible, and
 * strict.
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
		std::size_t index
	) const noexcept
	{
		best = load(value);
		where = static_cast<std::int64_t>(index);
	}

	template <typename Accumulator, typename T>
	void combine(
		Accumulator &best,
		std::int64_t &where,
		const T *value,
		std::size_t index
	) const noexcept
	{
		displace(
			best,
			where,
			static_cast<Accumulator>(load(value)),
			static_cast<std::int64_t>(index)
		);
	}

	template <typename Accumulator>
	void merge(
		Accumulator &best,
		std::int64_t &where,
		const Accumulator &other_best,
		const std::int64_t &other_where
	) const noexcept
	{
		displace(best, where, other_best, other_where);
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
	template <typename Accumulator>
	void displace(
		Accumulator &best,
		std::int64_t &where,
		const Accumulator &candidate,
		std::int64_t candidate_where
	) const noexcept
	{
		// Almost every candidate loses outright, so that is asked first, and
		// the index is only read for one that does not.
		if (
			!m_order(best, candidate) && 
			(m_order(candidate, best) || candidate_where < where)
		)
		{ 
			{
				best = candidate;
				where = candidate_where;
			}
		}
	}

	REXLIB_NO_UNIQUE_ADDRESS Order m_order;
};

} // namespace cpu
} // namespace rexlib
