// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/trigonometric/atanh_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

#include <cmath>
#include <complex>

namespace rex
{
namespace cpu
{

namespace
{

struct atanh_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		using std::atanh;
		store(result, atanh(load(x)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	atanh,
	ops::atanh_operation,
	default_kernel_factory<atanh_kernel>
);

} // namespace cpu
} // namespace rex
