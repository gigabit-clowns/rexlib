// SPDX-License-Identifier: GPL-3.0-only

#include "output_core_frame.hpp"

#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/operation/shape_rule/core_placement.hpp>

#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

namespace rexlib
{
namespace cpu
{

namespace detail
{

template <typename Op>
inline
named_core_frame make_output_core_frame(
	const Op &operation,
	std::vector<std::size_t> extents,
	std::integral_constant<core_placement, core_placement::named>
)
{
	return named_core_frame(
		std::move(extents),
		operation.get_parameters().get_axes()
	);
}

template <typename Op>
inline
named_core_frame make_output_core_frame(
	const Op& /*operation*/,
	std::vector<std::size_t> extents,
	std::integral_constant<core_placement, core_placement::trailing>
)
{
	using output_core_type = typename type_list_element<
		0,
		typename Op::shape_rule_type::output_core_list_type
	>::type;
	static_assert(
		!output_core_type::has_run,
		"Trailing core axes are split off by their number, so the core of "
		"the outputs must have a fixed number of axes."
	);

	return make_trailing_core_frame(
		std::move(extents),
		output_core_type::axis_count
	);
}

} // namespace detail

template <typename Op>
inline
named_core_frame make_output_core_frame(
	const Op &operation,
	span<const operand_descriptor> output_descriptors
)
{
	using placement_type = std::integral_constant<
		core_placement,
		Op::shape_rule_type::placement_type::placement
	>;

	std::vector<std::size_t> extents;
	output_descriptors[0].get_layout().get_extents(extents);
	return detail::make_output_core_frame(
		operation,
		std::move(extents),
		placement_type()
	);
}

} // namespace cpu
} // namespace rexlib
