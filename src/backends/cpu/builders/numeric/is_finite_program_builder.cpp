// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/numeric/is_finite_operation.hpp>

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

struct is_finite_kernel
{
	template <typename T>
	void operator()(bool *result, const T *x) const noexcept
	{
		using std::isfinite;
		store(result, isfinite(load(x)));
	}

	template <typename T>
	void operator()(bool *result, const std::complex<T> *x) const noexcept
	{
		using std::isfinite;

		const auto value = load(x);
		store(result, isfinite(value.real()) && isfinite(value.imag()));
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	is_finite,
	ops::is_finite_operation,
	default_kernel_factory<is_finite_kernel>
);

} // namespace cpu
} // namespace rexlib
