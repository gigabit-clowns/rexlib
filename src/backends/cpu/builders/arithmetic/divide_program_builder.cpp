// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/arithmetic/divide_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

namespace rexlib
{
namespace cpu
{

namespace
{

struct divide_kernel
{
	template <typename T>
	void operator()(T *result, const T *x, const T *y) const noexcept
	{
		store(result, load(x) / load(y));
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	divide,
	ops::divide_operation,
	default_kernel_factory<divide_kernel>
);

} // namespace cpu
} // namespace rexlib
