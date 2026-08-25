// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/arithmetic/multiply_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

namespace rexlib
{
namespace cpu
{

namespace
{

struct multiply_kernel
{
	template <typename T>
	void operator()(T *result, const T *x, const T *y) const noexcept
	{
		store(result, load(x) * load(y));
	}

	void operator()(bool *result, const bool *x, const bool *y) const noexcept
	{
		store(result, load(x) && load(y));
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	multiply,
	ops::multiply_operation,
	default_kernel_factory<multiply_kernel>
);

} // namespace cpu
} // namespace rexlib
