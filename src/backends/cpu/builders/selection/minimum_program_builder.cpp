// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/selection/minimum_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/kernels/ordering.hpp>
#include <backends/cpu/load_store.hpp>

namespace rex
{
namespace cpu
{

namespace
{

struct minimum_kernel
{
	template <typename T>
	void operator()(T *result, const T *x, const T *y) const noexcept
	{
		store(result, minimum_of(load(x), load(y)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	minimum,
	ops::minimum_operation,
	default_kernel_factory<minimum_kernel>
);

} // namespace cpu
} // namespace rex
