// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/trigonometric/atan_operation.hpp>

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

struct atan_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		using std::atan;
		store(result, atan(load(x)));
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	atan,
	ops::atan_operation,
	default_kernel_factory<atan_kernel>
);

} // namespace cpu
} // namespace rexlib
