// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/logical/logical_not_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/kernels/boolean_cast.hpp>
#include <backends/cpu/load_store.hpp>

#include <complex>

namespace rex
{
namespace cpu
{

namespace
{

struct logical_not_kernel
{
	template <typename T>
	void operator()(bool *result, const T *x) const noexcept
	{
		store(result, !to_boolean(load(x)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	logical_not,
	ops::logical_not_operation,
	default_kernel_factory<logical_not_kernel>
);

} // namespace cpu
} // namespace rex
