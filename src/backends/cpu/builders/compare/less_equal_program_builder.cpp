// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/compare/less_equal_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

namespace rex
{
namespace cpu
{

namespace
{

struct less_equal_kernel
{
	void operator()(bool *result, const bool *x, const bool *y) const noexcept
	{
		store(result, !load(x) || load(y));
	}

	template <typename T>
	void operator()(bool *result, const T *x, const T *y) const noexcept
	{
		store(result, load(x) <= load(y));
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	less_equal,
	ops::less_equal_operation,
	default_kernel_factory<less_equal_kernel>
);

} // namespace cpu
} // namespace rex
