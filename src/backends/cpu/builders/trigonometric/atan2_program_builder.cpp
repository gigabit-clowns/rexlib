// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/trigonometric/atan2_operation.hpp>

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

struct atan2_kernel
{
	template <typename T>
	void operator()(T *result, const T *y, const T *x) const noexcept
	{
		using std::atan2;
		store(result, atan2(load(y), load(x)));
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	atan2,
	ops::atan2_operation,
	default_kernel_factory<atan2_kernel>
);

} // namespace cpu
} // namespace rex
