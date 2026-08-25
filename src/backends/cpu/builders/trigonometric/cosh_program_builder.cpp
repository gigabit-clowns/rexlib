// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/trigonometric/cosh_operation.hpp>

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

struct cosh_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		using std::cosh;
		store(result, cosh(load(x)));
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	cosh,
	ops::cosh_operation,
	default_kernel_factory<cosh_kernel>
);

} // namespace cpu
} // namespace rexlib
