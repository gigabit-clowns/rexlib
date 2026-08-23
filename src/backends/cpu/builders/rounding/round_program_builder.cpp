// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/rounding/round_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

#include <cmath>

namespace xmipp4
{
namespace cpu
{

namespace
{

/**
 * @brief Round to the nearest integer, halves going to the even one.
 *
 * std::round sends a half away from zero, so it turns 0.5 into 1 and
 * accumulates a bias upwards over a distribution. NumPy rounds halves to
 * the even neighbour instead, which cancels that bias, and this follows
 * it.
 *
 * Written out rather than deferring to std::nearbyint or std::rint, both
 * of which follow the ambient floating point rounding mode and so would
 * make the operation's result depend on state the caller might have
 * changed.
 *
 * @tparam T The floating point element type.
 * @param value The value to round.
 * @return T The nearest integer, ties going to the even one.
 */
template <typename T>
T round_half_to_even(T value) noexcept
{
	using std::floor;
	using std::fmod;

	const auto lowered = floor(value);
	const auto fraction = value - lowered;

	if (fraction > T(0.5))
	{
		return lowered + T(1);
	}

	if (fraction < T(0.5))
	{
		// Also the path taken by a value too large to hold a fractional
		// part, which is already the integer it rounds to.
		return lowered;
	}

	// Exactly halfway: take whichever neighbour is even.
	//
	// A value that is not finite reaches here too, its fraction being not
	// a number and so comparing false against both bounds. The remainder
	// below is then not a number either, and adding one leaves an
	// infinity an infinity and a not-a-number what it was.
	return (fmod(lowered, T(2)) == T(0)) ? lowered : lowered + T(1);
}

struct round_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		store(result, round_half_to_even(load(x)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	round,
	ops::round_operation,
	default_kernel_factory<round_kernel>
);

} // namespace cpu
} // namespace xmipp4
