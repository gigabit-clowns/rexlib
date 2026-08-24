// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/rounding/floor_operation.hpp>

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

struct floor_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		using std::floor;
		store(result, floor(load(x)));
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	floor,
	ops::floor_operation,
	default_kernel_factory<floor_kernel>
);

} // namespace cpu
} // namespace rex
