// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/arithmetic/negate_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

namespace rexlib
{
namespace cpu
{

namespace
{

struct negate_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		store(result, -load(x));
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	negate,
	ops::negate_operation,
	default_kernel_factory<negate_kernel>
);

} // namespace cpu
} // namespace rexlib
