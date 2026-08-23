// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/trigonometric/acos_operation.hpp>

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

struct acos_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		using std::acos;
		store(result, acos(load(x)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	acos,
	ops::acos_operation,
	default_kernel_factory<acos_kernel>
);

} // namespace cpu
} // namespace rex
