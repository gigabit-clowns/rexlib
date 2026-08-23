// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/trigonometric/acosh_operation.hpp>

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

struct acosh_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		using std::acosh;
		store(result, acosh(load(x)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	acosh,
	ops::acosh_operation,
	default_kernel_factory<acosh_kernel>
);

} // namespace cpu
} // namespace rex
