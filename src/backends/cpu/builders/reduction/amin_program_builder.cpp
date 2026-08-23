// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/reduction/amin_operation.hpp>

#include <backends/cpu/builders/reduction_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/builders/fold_reduction_kernel.hpp>
#include <backends/cpu/kernels/ordering.hpp>

namespace xmipp4
{
namespace cpu
{

namespace
{

/**
 * @brief The smaller of two elements, an unordered one winning.
 *
 * No identity, deliberately. The largest representable value would serve as
 * one for every type this operation admits, but answering an empty reduction
 * with it would be inventing a smallest element where there is none. Leaving
 * it out makes the loop reject that case, which is the same answer NumPy
 * gives.
 */
struct minimum_fold
{
	template <typename T>
	T operator()(const T &accumulator, const T &value) const noexcept
	{
		return minimum_of(accumulator, value);
	}
};

} // anonymous namespace

REX_REGISTER_REDUCTION_PROGRAM_BUILDER(
	amin,
	ops::amin_operation,
	default_kernel_factory<fold_reduction_kernel<minimum_fold>>
);

} // namespace cpu
} // namespace xmipp4
