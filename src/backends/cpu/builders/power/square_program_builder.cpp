// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/power/square_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

#include <complex>

namespace rexlib
{
namespace cpu
{

namespace
{

struct square_kernel
{
	// Multiplying the element by itself rather than raising it to two
	// keeps the integer types exact and costs one multiplication.
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		const auto value = load(x);
		store(result, value * value);
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	square,
	ops::square_operation,
	default_kernel_factory<square_kernel>
);

} // namespace cpu
} // namespace rexlib
