// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/complex/conjugate_operation.hpp>

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

struct conjugate_kernel
{
	// A real number is its own conjugate. std::conj is not used here
	// because for a real argument it returns a complex, which is not the
	// type this operation promises.
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		store(result, load(x));
	}

	template <typename T>
	void operator()(
		std::complex<T> *result,
		const std::complex<T> *x
	) const noexcept
	{
		store(result, std::conj(load(x)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	conjugate,
	ops::conjugate_operation,
	default_kernel_factory<conjugate_kernel>
);

} // namespace cpu
} // namespace rex
