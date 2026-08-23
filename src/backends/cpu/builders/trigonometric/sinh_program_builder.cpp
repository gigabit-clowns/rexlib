// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/trigonometric/sinh_operation.hpp>

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

struct sinh_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		using std::sinh;
		store(result, sinh(load(x)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	sinh,
	ops::sinh_operation,
	default_kernel_factory<sinh_kernel>
);

} // namespace cpu
} // namespace xmipp4
