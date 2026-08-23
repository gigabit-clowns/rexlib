// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/bitwise/right_shift_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

namespace xmipp4
{
namespace cpu
{

namespace
{

struct right_shift_kernel
{
	template <typename T>
	void operator()(T *result, const T *value, const T *count) const noexcept
	{
		store(result, static_cast<T>(load(value) >> load(count)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	right_shift,
	ops::right_shift_operation,
	default_kernel_factory<right_shift_kernel>
);

} // namespace cpu
} // namespace xmipp4
