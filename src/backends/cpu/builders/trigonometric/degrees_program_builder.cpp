// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/trigonometric/degrees_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

namespace rex
{
namespace cpu
{

namespace
{

// Spelled out rather than taken from M_PI, which is POSIX rather than
// standard C++ and needs _USE_MATH_DEFINES before <cmath> on MSVC. It
// rounds to the same double, and the ratio is formed once per element
// type rather than inherited from one rounded for another.
const long double pi = 3.141592653589793238462643383279502884L;

struct degrees_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		const auto value = load(x);
		using compute_type = decltype(value);
		store(result, value * static_cast<compute_type>(180.0L / pi));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	degrees,
	ops::degrees_operation,
	default_kernel_factory<degrees_kernel>
);

} // namespace cpu
} // namespace rex
