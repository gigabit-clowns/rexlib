// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/trigonometric/degrees_operation.hpp>

#include <backends/cpu/skeletons/elementwise/elementwise_program_builder.hpp>
#include <backends/cpu/skeletons/default_kernel_factory.hpp>
#include <backends/cpu/numerical/load_store.hpp>

#include <boost/math/constants/constants.hpp>

namespace rexlib
{
namespace cpu
{

namespace
{

struct degrees_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		using compute_type = typename element_compute_type<T>::type;
		const auto value = load(x);
		store(result, value * boost::math::constants::radian<compute_type>());
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	degrees,
	ops::degrees_operation,
	default_kernel_factory<degrees_kernel>
);

} // namespace cpu
} // namespace rexlib
