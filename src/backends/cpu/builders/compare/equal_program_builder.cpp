// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/compare/equal_operation.hpp>

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

struct equal_kernel
{
	template <typename T>
	void operator()(bool *result, const T *x, const T *y) const noexcept
	{
		store(result, load(x) == load(y));
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	equal,
	ops::equal_operation,
	default_kernel_factory<equal_kernel>
);

} // namespace cpu
} // namespace rex
