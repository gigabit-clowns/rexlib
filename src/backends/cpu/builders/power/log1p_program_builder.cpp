// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/power/log1p_operation.hpp>

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

struct log1p_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		using std::log1p;
		store(result, log1p(load(x)));
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	log1p,
	ops::log1p_operation,
	default_kernel_factory<log1p_kernel>
);

} // namespace cpu
} // namespace rexlib
