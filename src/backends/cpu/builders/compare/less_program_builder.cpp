// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/compare/less_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

namespace rexlib
{
namespace cpu
{

namespace
{

struct less_kernel
{
	void operator()(bool *result, const bool *x, const bool *y) const noexcept
	{
		store(result, !load(x) && load(y));
	}

	template <typename T>
	void operator()(bool *result, const T *x, const T *y) const noexcept
	{
		store(result, load(x) < load(y));
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	less,
	ops::less_operation,
	default_kernel_factory<less_kernel>
);

} // namespace cpu
} // namespace rexlib
