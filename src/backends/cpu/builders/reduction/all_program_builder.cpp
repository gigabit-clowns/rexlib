// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/reduction/all_operation.hpp>

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

struct conjunction_fold
{
	bool operator()(bool accumulator, bool value) const noexcept
	{
		return accumulator && value;
	}

	// A conjunction of nothing holds of everything.
	template <typename T>
	static T identity() noexcept
	{
		return true;
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
using all_kernel = fold_reduction_kernel<conjunction_fold, boolean_lift>;

} // anonymous namespace

REX_REGISTER_REDUCTION_PROGRAM_BUILDER(
	all,
	ops::all_operation,
	default_kernel_factory<all_kernel>
);

} // namespace cpu
} // namespace xmipp4
