// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/reduction/amax_operation.hpp>

#include <backends/cpu/builders/reduction_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/builders/fold_reduction_kernel.hpp>
#include <backends/cpu/kernels/ordering.hpp>

namespace rex
{
namespace cpu
{

namespace
{

/**
 * @brief The larger of two elements, an unordered one winning.
 *
 * No identity, deliberately. The lowest representable value would serve as
 * one for every type this operation admits, but answering an empty reduction
 * with it would be inventing a largest element where there is none. Leaving
 * it out makes the loop reject that case, which is the same answer NumPy
 * gives.
 */
struct maximum_fold
{
	template <typename T>
	T operator()(const T &accumulator, const T &value) const noexcept
	{
		return maximum_of(accumulator, value);
	}
};

} // anonymous namespace

REX_REGISTER_REDUCTION_PROGRAM_BUILDER(
	amax,
	ops::amax_operation,
	default_kernel_factory<fold_reduction_kernel<maximum_fold>>
);

} // namespace cpu
} // namespace rex
