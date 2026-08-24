// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/power/reciprocal_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

#include <complex>

namespace rex
{
namespace cpu
{

namespace
{

struct reciprocal_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		const auto value = load(x);

		// The one is built in the compute type so that a complex operand
		// divides a complex one rather than a scalar.
		using compute_type = decltype(value);
		store(result, compute_type(1) / value);
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	reciprocal,
	ops::reciprocal_operation,
	default_kernel_factory<reciprocal_kernel>
);

} // namespace cpu
} // namespace rex
