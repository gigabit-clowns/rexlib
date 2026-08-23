// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/power/log10_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

#include <cmath>
#include <complex>

namespace xmipp4
{
namespace cpu
{

namespace
{

struct log10_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		using std::log10;
		store(result, log10(load(x)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	log10,
	ops::log10_operation,
	default_kernel_factory<log10_kernel>
);

} // namespace cpu
} // namespace xmipp4
