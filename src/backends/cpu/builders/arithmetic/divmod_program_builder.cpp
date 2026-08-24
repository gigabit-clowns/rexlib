// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/arithmetic/divmod_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/kernels/floor_division.hpp>
#include <backends/cpu/load_store.hpp>

namespace rexlib
{
namespace cpu
{

namespace
{

struct divmod_kernel
{
	// Both results come from one pass over the operands, which is the
	// whole point of the operation existing beside floor_divide and
	// modulo rather than being spelled as the two of them.
	template <typename T>
	void operator()(
		T *quotient,
		T *remainder,
		const T *x,
		const T *y
	) const noexcept
	{
		const auto dividend = load(x);
		const auto divisor = load(y);
		store(quotient, floor_div(dividend, divisor));
		store(remainder, floor_mod(dividend, divisor));
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	divmod,
	ops::divmod_operation,
	default_kernel_factory<divmod_kernel>
);

} // namespace cpu
} // namespace rexlib
