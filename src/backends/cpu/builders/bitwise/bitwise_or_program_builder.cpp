// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/bitwise/bitwise_or_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

namespace rexlib
{
namespace cpu
{

namespace
{

struct bitwise_or_kernel
{
	template <typename T>
	void operator()(T *result, const T *x, const T *y) const noexcept
	{
		store(result, static_cast<T>(load(x) | load(y)));
	}

	void operator()(bool *result, const bool *x, const bool *y) const noexcept
	{
		store(result, load(x) || load(y));
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	bitwise_or,
	ops::bitwise_or_operation,
	default_kernel_factory<bitwise_or_kernel>
);

} // namespace cpu
} // namespace rexlib
