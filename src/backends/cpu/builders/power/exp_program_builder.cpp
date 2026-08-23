// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/power/exp_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

#include <cmath>
#include <complex>

namespace xmipp4
{
namespace cpu
{

namespace
{

struct exp_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		using std::exp;
		store(result, exp(load(x)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	exp,
	ops::exp_operation,
	default_kernel_factory<exp_kernel>
);

} // namespace cpu
} // namespace xmipp4
