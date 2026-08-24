// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/meta/type_list.hpp>
#include <rex/core/platform/constexpr.hpp>
#include <rex/core/platform/cpp_attributes.hpp>

#include <backends/cpu/builders/reduction_compute_type.hpp>
#include <backends/cpu/load_store.hpp>

#include <cstddef>
#include <type_traits>
#include <utility>

namespace rex
{
namespace cpu
{

// This header holds scaffolding rather than arithmetic: it adapts a plain
// binary fold to the reduction kernel concept, the way default_kernel_factory
// adapts a stateless kernel to the factory one. That is why it sits beside
// the builders and not in kernels/, which is for the arithmetic itself: a
// fold's own body belongs in its operation's translation unit, or in kernels/
// when several operations share it.

/**
 * @brief The default way an element enters an accumulator.
 *
 * A plain conversion, which is what a fold over the operand's own type
 * needs. An operation answering about its elements rather than with them,
 * such as one counting or testing them, supplies its own instead.
 */
struct reduction_conversion_lift
{
	template <typename Accumulator, typename T>
	static Accumulator apply(const T &value) noexcept
	{
		return static_cast<Accumulator>(value);
	}
};

/**
 * @brief Reduction kernel built from a binary fold.
 *
 * Covers the operations whose whole definition is one associative binary
 * operation with a neutral element, optionally preceded by a transformation
 * of each element on its way into the accumulator. The accumulator is the
 * computation type of the output, so the declared typing rule remains the
 * only thing deciding what a reduction accumulates in.
 *
 * A fold is invoked as `fold(accumulator, value)` and must additionally
 * provide `Fold::identity<T>()`, without which a reduction over no elements
 * is rejected rather than answered. An operation needing a post processing
 * on the way out derives from this and hides `finalize`; one needing more
 * than one accumulator spells its members out itself.
 *
 * @tparam Fold The binary fold. Must be default constructible.
 * @tparam Lift How an element enters the accumulator, invoked as
 * `Lift::apply<Accumulator>(value)`.
 */
template <typename Fold, typename Lift = reduction_conversion_lift>
class fold_reduction_kernel
{
public:
	/**
	 * @brief The fold may be dealt out over lanes.
	 *
	 * An associative binary operation is what this class is for, so a run may
	 * be folded in any grouping, and a fold answering with a value rather
	 * than with a place does not care which elements a lane was dealt. Both
	 * are what @ref has_reassociable_fold asks for.
	 *
	 * What it costs is exactness: a fold over inexact arithmetic stops being
	 * bit for bit what a single accumulator arrives at. That is the same
	 * trade the threaded fold split already makes, and it is what keeps a
	 * reduction along the contiguous axis from being one chain of dependent
	 * operations.
	 */
	static REXLIB_CONST_CONSTEXPR bool reassociable_fold = true;

	/**
	 * @brief A single accumulator, of the output's computation type.
	 *
	 * @tparam Outputs type_list of the output element types.
	 * @tparam Inputs type_list of the input element types.
	 */
	template <typename Outputs, typename Inputs>
	struct accumulators
	{
		using type = type_list<
			typename reduction_compute_type<
				typename type_list_element<0, Outputs>::type
			>::type
		>;
	};

	// The trailing position says where in the reduced space the element
	// sits. Only an operation reporting a location has any use for it, so
	// it is ignored here and the compiler drops the arithmetic behind it.
	template <typename Accumulator, typename T>
	void seed(
		Accumulator &accumulator,
		const T *value,
		std::size_t /*position*/
	) const noexcept
	{
		accumulator = Lift::template apply<Accumulator>(load(value));
	}

	template <typename Accumulator, typename T>
	void combine(
		Accumulator &accumulator,
		const T *value,
		std::size_t /*position*/
	) const noexcept
	{
		accumulator = m_fold(
			accumulator,
			Lift::template apply<Accumulator>(load(value))
		);
	}

	template <typename Accumulator>
	void merge(
		Accumulator &accumulator,
		const Accumulator &other
	) const noexcept
	{
		accumulator = m_fold(accumulator, other);
	}

	template <typename U, typename Accumulator>
	void finalize(
		U *result,
		const Accumulator &accumulator,
		std::size_t /*count*/
	) const noexcept
	{
		store(result, accumulator);
	}

	/**
	 * @brief The neutral element of the fold, when it has one.
	 *
	 * The return type is spelled in terms of the fold's own identity so that
	 * this member disappears for a fold without one, rather than failing to
	 * compile when the loop asks for it. A reduction over no elements is
	 * then rejected at the one place that can tell the difference.
	 *
	 * The fold is bound to a parameter of this member rather than being read
	 * from the enclosing class, so that the lookup happens while the member
	 * is being substituted and a missing identity is a deduction failure
	 * instead of an error.
	 */
	template <typename Accumulator, typename F = Fold>
	auto identity(Accumulator &accumulator) const noexcept
		-> decltype(static_cast<void>(F::template identity<Accumulator>()))
	{
		accumulator = F::template identity<Accumulator>();
	}

private:
	REXLIB_NO_UNIQUE_ADDRESS Fold m_fold;
};

} // namespace cpu
} // namespace rex
