// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/bitwise/bitwise_not_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

namespace rex
{
namespace cpu
{

namespace
{

struct bitwise_not_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		store(result, static_cast<T>(~load(x)));
	}

	void operator()(bool *result, const bool *x) const noexcept
	{
		store(result, !load(x));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	bitwise_not,
	ops::bitwise_not_operation,
	default_kernel_factory<bitwise_not_kernel>
);

} // namespace cpu
} // namespace rex
