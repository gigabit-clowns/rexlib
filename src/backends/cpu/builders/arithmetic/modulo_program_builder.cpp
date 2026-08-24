// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/arithmetic/modulo_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/kernels/floor_division.hpp>
#include <backends/cpu/load_store.hpp>

namespace rexlib
{
namespace cpu
{

namespace
{

struct modulo_kernel
{
	template <typename T>
	void operator()(T *result, const T *x, const T *y) const noexcept
	{
		store(result, floor_mod(load(x), load(y)));
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	modulo,
	ops::modulo_operation,
	default_kernel_factory<modulo_kernel>
);

} // namespace cpu
} // namespace rexlib
