// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/reduction/count_nonzero_operation.hpp>

#include <backends/cpu/builders/reduction_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/builders/fold_reduction_kernel.hpp>
#include <backends/cpu/kernels/boolean_cast.hpp>

namespace xmipp4
{
namespace cpu
{

namespace
{

struct sum_fold
{
	template <typename T>
	T operator()(const T &accumulator, const T &value) const noexcept
	{
		return accumulator + value;
	}

	template <typename T>
	static T identity() noexcept
	{
		return T(0);
	}
};

/**
 * @brief Each element enters the total as one or as nothing.
 *
 * The operation answers about its elements rather than with them, so what
 * is accumulated is not the element but the answer to whether it is set.
 * That is the whole of the difference between counting and summing.
 */
struct nonzero_lift
{
	template <typename Accumulator, typename T>
	static Accumulator apply(const T &value) noexcept
	{
		return to_boolean(value) ? Accumulator(1) : Accumulator(0);
	}
};

// Named rather than spelled inline: the kernel is two template
// arguments, and a comma inside a macro argument is not one.
using count_nonzero_kernel = fold_reduction_kernel<sum_fold, nonzero_lift>;

} // anonymous namespace

REX_REGISTER_REDUCTION_PROGRAM_BUILDER(
	count_nonzero,
	ops::count_nonzero_operation,
	default_kernel_factory<count_nonzero_kernel>
);

} // namespace cpu
} // namespace xmipp4
