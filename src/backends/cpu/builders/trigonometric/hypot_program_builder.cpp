// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/trigonometric/hypot_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

#include <cmath>

namespace rexlib
{
namespace cpu
{

namespace
{

struct hypot_kernel
{
	template <typename T>
	void operator()(T *result, const T *x, const T *y) const noexcept
	{
		using std::hypot;
		store(result, hypot(load(x), load(y)));
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	hypot,
	ops::hypot_operation,
	default_kernel_factory<hypot_kernel>
);

} // namespace cpu
} // namespace rexlib
