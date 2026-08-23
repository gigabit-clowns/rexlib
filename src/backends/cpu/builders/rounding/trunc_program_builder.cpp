// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/rounding/trunc_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

#include <cmath>

namespace rex
{
namespace cpu
{

namespace
{

struct trunc_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		using std::trunc;
		store(result, trunc(load(x)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	trunc,
	ops::trunc_operation,
	default_kernel_factory<trunc_kernel>
);

} // namespace cpu
} // namespace rex
