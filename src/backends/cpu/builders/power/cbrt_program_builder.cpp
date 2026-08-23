// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/power/cbrt_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

#include <cmath>

namespace rex
{
namespace cpu
{

namespace
{

struct cbrt_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		using std::cbrt;
		store(result, cbrt(load(x)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	cbrt,
	ops::cbrt_operation,
	default_kernel_factory<cbrt_kernel>
);

} // namespace cpu
} // namespace rex
