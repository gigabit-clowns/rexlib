// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/arithmetic/abs_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

#include <complex>
#include <type_traits>

namespace rex
{
namespace cpu
{

namespace
{

struct abs_kernel
{
	template <typename T>
	typename std::enable_if<std::is_unsigned<T>::value, void>::type
	operator()(T *result, const T *x) const noexcept
	{
		store(result, load(x));
	}

	template <typename T>
	typename std::enable_if<!std::is_unsigned<T>::value, void>::type
	operator()(T *result, const T *x) const noexcept
	{
		using std::abs;
		store(result, abs(load(x)));
	}

	template <typename T>
	void operator()(T *result, const std::complex<T> *x) const noexcept
	{
		store(result, std::abs(load(x)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	abs,
	ops::abs_operation,
	default_kernel_factory<abs_kernel>
);

} // namespace cpu
} // namespace rex
