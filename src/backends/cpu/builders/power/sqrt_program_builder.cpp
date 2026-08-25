// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/power/sqrt_operation.hpp>

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

struct sqrt_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		using std::sqrt;
		store(result, sqrt(load(x)));
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	sqrt,
	ops::sqrt_operation,
	default_kernel_factory<sqrt_kernel>
);

} // namespace cpu
} // namespace rexlib
