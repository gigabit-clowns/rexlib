// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/linalg/vecdot_operation.hpp>

#include <backends/cpu/builders/reduction_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/builders/reduction_compute_type.hpp>
#include <backends/cpu/load_store.hpp>

#include <rex/core/meta/type_list.hpp>

#include <complex>
#include <cstddef>

namespace rexlib
{
namespace cpu
{

namespace
{

/**
 * @brief The contribution one pair of elements makes to the contraction.
 *
 * The first operand is conjugated, which is what makes a vector contracted
 * with itself its squared magnitude rather than a complex number, and is
 * what the array API and NumPy's vecdot both do. For real operands the
 * conjugation is the identity, so this is a plain product everywhere else.
 */
template <typename T>
T contract(const T &left, const T &right) noexcept
{
	return left * right;
}

template <typename T>
std::complex<T> contract(
	const std::complex<T> &left,
	const std::complex<T> &right
) noexcept
{
	return std::conj(left) * right;
}

/**
 * @brief Sum the products of two operands along the contracted axis.
 *
 * The only reduction taking two inputs, which is why it spells its members
 * out rather than using the fold adaptor: what enters the accumulator is
 * built from a pair of elements, not from one.
 */
struct vecdot_kernel
{
	template <typename Outputs, typename Inputs>
	struct accumulators
	{
		using type = type_list<
			typename reduction_compute_type<
				typename type_list_element<0, Outputs>::type
			>::type
		>;
	};

	template <typename Accumulator, typename T>
	void seed(
		Accumulator &accumulator,
		const T *left,
		const T *right,
		std::size_t /*position*/
	) const noexcept
	{
		accumulator = contract(load(left), load(right));
	}

	template <typename Accumulator, typename T>
	void combine(
		Accumulator &accumulator,
		const T *left,
		const T *right,
		std::size_t /*position*/
	) const noexcept
	{
		accumulator += contract(load(left), load(right));
	}

	template <typename Accumulator>
	void merge(
		Accumulator &accumulator,
		const Accumulator &other
	) const noexcept
	{
		accumulator += other;
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

	// A contraction over no elements is an empty sum.
	template <typename Accumulator>
	void identity(Accumulator &accumulator) const noexcept
	{
		accumulator = Accumulator(0);
	}
};

} // anonymous namespace

REXLIB_REGISTER_REDUCTION_PROGRAM_BUILDER(
	vecdot,
	ops::vecdot_operation,
	default_kernel_factory<vecdot_kernel>
);

} // namespace cpu
} // namespace rexlib
