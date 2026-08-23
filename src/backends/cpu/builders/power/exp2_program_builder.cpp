// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/power/exp2_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

#include <cmath>

namespace xmipp4
{
namespace cpu
{

namespace
{

struct exp2_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		using std::exp2;
		store(result, exp2(load(x)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	exp2,
	ops::exp2_operation,
	default_kernel_factory<exp2_kernel>
);

} // namespace cpu
} // namespace xmipp4
