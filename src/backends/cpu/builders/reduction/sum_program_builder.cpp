// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/reduction/sum_operation.hpp>

#include <backends/cpu/builders/reduction_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/builders/fold_reduction_kernel.hpp>

namespace rexlib
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

	// Booleans add the only way their own type can hold, which is the same
	// choice the elementwise addition makes.
	bool operator()(bool accumulator, bool value) const noexcept
	{
		return accumulator || value;
	}

	template <typename T>
	static T identity() noexcept
	{
		return T(0);
	}
};

} // anonymous namespace

REXLIB_REGISTER_REDUCTION_PROGRAM_BUILDER(
	sum,
	ops::sum_operation,
	default_kernel_factory<fold_reduction_kernel<sum_fold>>
);

} // namespace cpu
} // namespace rexlib
