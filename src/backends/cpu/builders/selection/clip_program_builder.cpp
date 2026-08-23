// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/selection/clip_operation.hpp>

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

struct clip_kernel
{
	template <typename T>
	void operator()(
		T *result,
		const T *value,
		const T *lower,
		const T *upper
	) const noexcept
	{
		const auto bounded_below = maximum_of(load(value), load(lower));
		store(result, minimum_of(bounded_below, load(upper)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	clip,
	ops::clip_operation,
	default_kernel_factory<clip_kernel>
);

} // namespace cpu
} // namespace rex
