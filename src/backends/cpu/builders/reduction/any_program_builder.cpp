// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/reduction/any_operation.hpp>

#include <backends/cpu/builders/reduction_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/builders/fold_reduction_kernel.hpp>
#include <backends/cpu/kernels/boolean_cast.hpp>

namespace rexlib
{
namespace cpu
{

namespace
{

struct disjunction_fold
{
	bool operator()(bool accumulator, bool value) const noexcept
	{
		return accumulator || value;
	}

	// A disjunction of nothing holds of nothing.
	template <typename T>
	static T identity() noexcept
	{
		return false;
	}
};

/**
 * @brief Each element enters as the answer to whether it is set.
 */
struct boolean_lift
{
	template <typename Accumulator, typename T>
	static Accumulator apply(const T &value) noexcept
	{
		return to_boolean(value);
	}
};

// Named rather than spelled inline: the kernel is two template
// arguments, and a comma inside a macro argument is not one.
using any_kernel = fold_reduction_kernel<disjunction_fold, boolean_lift>;

} // anonymous namespace

REXLIB_REGISTER_REDUCTION_PROGRAM_BUILDER(
	any,
	ops::any_operation,
	default_kernel_factory<any_kernel>
);

} // namespace cpu
} // namespace rexlib
