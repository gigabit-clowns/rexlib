// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/compare/greater_equal_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

namespace rex
{
namespace cpu
{

namespace
{

struct greater_equal_kernel
{
	void operator()(bool *result, const bool *x, const bool *y) const noexcept
	{
		store(result, load(x) || !load(y));
	}

	template <typename T>
	void operator()(bool *result, const T *x, const T *y) const noexcept
	{
		store(result, load(x) >= load(y));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	greater_equal,
	ops::greater_equal_operation,
	default_kernel_factory<greater_equal_kernel>
);

} // namespace cpu
} // namespace rex
