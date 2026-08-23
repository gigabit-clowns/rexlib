// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/trigonometric/sincos_operation.hpp>

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

struct sincos_kernel
{
	// One load of the angle serves both results, which is the reason the
	// operation exists beside sin and cos rather than being spelled as
	// the two of them.
	template <typename T>
	void operator()(T *sine, T *cosine, const T *x) const noexcept
	{
		using std::sin;
		using std::cos;

		const auto angle = load(x);
		store(sine, sin(angle));
		store(cosine, cos(angle));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	sincos,
	ops::sincos_operation,
	default_kernel_factory<sincos_kernel>
);

} // namespace cpu
} // namespace rex
