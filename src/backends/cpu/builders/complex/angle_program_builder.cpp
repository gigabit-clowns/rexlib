// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/complex/angle_operation.hpp>

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

struct angle_kernel
{
	// std::arg is defined for the real types as well, where it answers
	// for the point on the real axis: zero to the right of the origin and
	// half a turn to the left of it. That is what NumPy reports, so both
	// overloads compute the same way and differ only in the operand they
	// take, the result being real either way.
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		store(result, std::arg(load(x)));
	}

	template <typename T>
	void operator()(T *result, const std::complex<T> *x) const noexcept
	{
		store(result, std::arg(load(x)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	angle,
	ops::angle_operation,
	default_kernel_factory<angle_kernel>
);

} // namespace cpu
} // namespace rex
