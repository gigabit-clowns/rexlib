// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/logical/logical_or_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/kernels/boolean_cast.hpp>
#include <backends/cpu/load_store.hpp>

#include <complex>

namespace xmipp4
{
namespace cpu
{

namespace
{

struct logical_or_kernel
{
	template <typename T>
	void operator()(bool *result, const T *x, const T *y) const noexcept
	{
		store(result, to_boolean(load(x)) || to_boolean(load(y)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	logical_or,
	ops::logical_or_operation,
	default_kernel_factory<logical_or_kernel>
);

} // namespace cpu
} // namespace xmipp4
