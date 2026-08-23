// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/compare/equal_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

#include <complex>

namespace xmipp4
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

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	equal,
	ops::equal_operation,
	default_kernel_factory<equal_kernel>
);

} // namespace cpu
} // namespace xmipp4
