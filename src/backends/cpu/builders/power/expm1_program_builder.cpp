// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/power/expm1_operation.hpp>

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

struct expm1_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		using std::expm1;
		store(result, expm1(load(x)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	expm1,
	ops::expm1_operation,
	default_kernel_factory<expm1_kernel>
);

} // namespace cpu
} // namespace rex
