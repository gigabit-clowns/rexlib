// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/compare/less_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

namespace xmipp4
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

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	less,
	ops::less_operation,
	default_kernel_factory<less_kernel>
);

} // namespace cpu
} // namespace xmipp4
