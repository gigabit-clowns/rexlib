// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/trigonometric/sin_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

#include <cmath>
#include <complex>

namespace rex
{
namespace cpu
{

namespace
{

struct sin_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		using std::sin;
		store(result, sin(load(x)));
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	sin,
	ops::sin_operation,
	default_kernel_factory<sin_kernel>
);

} // namespace cpu
} // namespace rex
