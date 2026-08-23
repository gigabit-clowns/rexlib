// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/trigonometric/asin_operation.hpp>

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

struct asin_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		using std::asin;
		store(result, asin(load(x)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	asin,
	ops::asin_operation,
	default_kernel_factory<asin_kernel>
);

} // namespace cpu
} // namespace xmipp4
