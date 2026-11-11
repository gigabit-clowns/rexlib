// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/trigonometric/radians_operation.hpp>

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

struct radians_kernel
{
	template <typename T>
	void operator()(T *result, const T *x) const noexcept
	{
		using compute_type = typename element_compute_type<T>::type;
		const auto value = load(x);
		store(result, value * boost::math::constants::degree<compute_type>());
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	radians,
	ops::radians_operation,
	default_kernel_factory<radians_kernel>
);

} // namespace cpu
} // namespace rexlib
