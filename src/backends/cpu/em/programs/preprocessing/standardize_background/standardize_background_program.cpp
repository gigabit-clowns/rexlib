// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/ops/preprocessing/standardize_background_operation.hpp>

#include <backends/cpu/math/geometry/centred_ball.hpp>
#include "standardize_background_kernel.hpp"

#include <backends/cpu/skeletons/batch_reduce_map/batch_reduce_map_program_builder.hpp>
#include <backends/cpu/skeletons/rule_type_dispatch.hpp>
#include <backends/cpu/loops/indices/element_index_tags.hpp>
#include <backends/cpu/skeletons/batch_reduce_map/batch_reduce_map_plan.hpp>

#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/platform/constexpr.hpp>

#include <cstddef>

namespace rexlib
{
namespace cpu
{

namespace
{

struct standardize_background_kernel_factory
{
	static REXLIB_CONST_CONSTEXPR std::size_t statistic_count = 2;
	static REXLIB_CONST_CONSTEXPR std::size_t pass_count = 2;

	template <typename... Outs, typename... Ins>
	standardize_background_kernel operator()(
		const em::standardize_background_operation &operation,
		type_list<Outs...>,
		type_list<Ins...>,
		const batch_reduce_map_plan &plan
	) const
	{
		const auto extents = plan.get_frame().get_core_extents();
		return standardize_background_kernel(
			centred_ball(make_span(extents), operation.get_radius()),
			operation.get_lower_bound(),
			operation.get_upper_bound()
		);
	}
};

} // anonymous namespace

REXLIB_REGISTER_BATCH_REDUCE_MAP_PROGRAM_BUILDER_EX(
	standardize_background,
	em::standardize_background_operation,
	standardize_background_kernel_factory,
	rule_type_dispatch<em::standardize_background_operation::type_rule_type>,
	multidimensional_index_tag,
	linear_index_tag
);

} // namespace cpu
} // namespace rexlib
