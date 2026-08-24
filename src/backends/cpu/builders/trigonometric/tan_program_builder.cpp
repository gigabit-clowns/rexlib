// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/trigonometric/tan_operation.hpp>

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

struct tan_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		using std::tan;
		store(result, tan(load(x)));
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	tan,
	ops::tan_operation,
	default_kernel_factory<tan_kernel>
);

} // namespace cpu
} // namespace rexlib
