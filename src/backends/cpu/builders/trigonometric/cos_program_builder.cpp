// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/trigonometric/cos_operation.hpp>

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

struct cos_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		using std::cos;
		store(result, cos(load(x)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	cos,
	ops::cos_operation,
	default_kernel_factory<cos_kernel>
);

} // namespace cpu
} // namespace rex
