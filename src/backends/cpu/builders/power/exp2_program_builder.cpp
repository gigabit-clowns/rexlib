// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/power/exp2_operation.hpp>

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

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	exp2,
	ops::exp2_operation,
	default_kernel_factory<exp2_kernel>
);

} // namespace cpu
} // namespace rex
