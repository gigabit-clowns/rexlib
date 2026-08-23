// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/arithmetic/floor_divide_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/kernels/floor_division.hpp>
#include <backends/cpu/load_store.hpp>

namespace rex
{
namespace cpu
{

namespace
{

struct floor_divide_kernel
{
	template <typename T>
	void operator()(T *result, const T *x, const T *y) const noexcept
	{
		store(result, floor_div(load(x), load(y)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	floor_divide,
	ops::floor_divide_operation,
	default_kernel_factory<floor_divide_kernel>
);

} // namespace cpu
} // namespace rex
