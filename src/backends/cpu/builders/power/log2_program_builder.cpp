// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/power/log2_operation.hpp>

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

struct log2_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		using std::log2;
		store(result, log2(load(x)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	log2,
	ops::log2_operation,
	default_kernel_factory<log2_kernel>
);

} // namespace cpu
} // namespace rex
