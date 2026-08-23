// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/numeric/sign_bit_operation.hpp>

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

struct sign_bit_kernel
{
	template <typename T>
	void operator()(bool *result, const T *x) const noexcept
	{
		using std::signbit;
		store(result, signbit(load(x)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	sign_bit,
	ops::sign_bit_operation,
	default_kernel_factory<sign_bit_kernel>
);

} // namespace cpu
} // namespace rex
