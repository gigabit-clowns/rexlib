// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/trigonometric/asinh_operation.hpp>

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

struct asinh_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		using std::asinh;
		store(result, asinh(load(x)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	asinh,
	ops::asinh_operation,
	default_kernel_factory<asinh_kernel>
);

} // namespace cpu
} // namespace xmipp4
