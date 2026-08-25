// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/power/exp_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

#include <cmath>
#include <complex>

namespace rexlib
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

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	exp,
	ops::exp_operation,
	default_kernel_factory<exp_kernel>
);

} // namespace cpu
} // namespace rexlib
