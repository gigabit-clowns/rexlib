// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/rounding/ceil_operation.hpp>

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

struct ceil_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		using std::ceil;
		store(result, ceil(load(x)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	ceil,
	ops::ceil_operation,
	default_kernel_factory<ceil_kernel>
);

} // namespace cpu
} // namespace rex
