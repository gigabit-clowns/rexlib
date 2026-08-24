// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/arithmetic/sign_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

#include <cmath>
#include <complex>
#include <type_traits>

namespace rexlib
{
namespace cpu
{

namespace
{

struct sign_kernel
{
	void operator()(bool *result, const bool *x) const noexcept
	{
		store(result, load(x));
	}

	template <typename T>
	typename std::enable_if<
		std::is_integral<T>::value && std::is_unsigned<T>::value,
		void
	>::type
	operator()(T *result, const T *x) const noexcept
	{
		store(result, static_cast<T>(load(x) > T(0) ? 1 : 0));
	}

	template <typename T>
	typename std::enable_if<
		std::is_integral<T>::value && std::is_signed<T>::value,
		void
	>::type
	operator()(T *result, const T *x) const noexcept
	{
		const auto value = load(x);
		store(result, static_cast<T>((T(0) < value) - (value < T(0))));
	}

	template <typename T>
	typename std::enable_if<!std::is_integral<T>::value, void>::type
	operator()(T *result, const T *x) const noexcept
	{
		using std::isnan;

		const auto value = load(x);
		using compute_type = decltype(value);

		if (isnan(value))
		{
			store(result, value);
		}
		else
		{
			store(
				result,
				static_cast<compute_type>(
					(compute_type(0) < value) - (value < compute_type(0))
				)
			);
		}
	}

	// The sign of a complex number is the unit vector along it, as the
	// operation declares. NumPy reported the sign of the real part until
	// 2.0 changed it to this, so a reference implementation older than
	// that will disagree here and only here.
	template <typename T>
	void operator()(
		std::complex<T> *result,
		const std::complex<T> *x
	) const noexcept
	{
		const auto value = load(x);
		const auto magnitude = std::abs(value);

		if (magnitude == 0)
		{
			store(result, value);
		}
		else
		{
			store(result, value / magnitude);
		}
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	sign,
	ops::sign_operation,
	default_kernel_factory<sign_kernel>
);

} // namespace cpu
} // namespace rexlib
