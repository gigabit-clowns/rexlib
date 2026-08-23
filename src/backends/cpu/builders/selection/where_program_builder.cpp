// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/selection/where_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

#include <complex>

namespace xmipp4
{
namespace cpu
{

namespace
{

struct where_kernel
{
	template <typename T>
	void operator()(
		T *result,
		const bool *condition,
		const T *x,
		const T *y
	) const noexcept
	{
		store(result, load(condition) ? load(x) : load(y));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	where,
	ops::where_operation,
	default_kernel_factory<where_kernel>
);

} // namespace cpu
} // namespace xmipp4
