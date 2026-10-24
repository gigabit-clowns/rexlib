// SPDX-License-Identifier: GPL-3.0-only

#include "shape_rule_descriptor_of.hpp"

#include <rexlib/core/operation/shape_rule/core_dimensions_descriptor_table.hpp>
#include <rexlib/core/operation/shape_rule/extent_pivot_descriptor_table.hpp>

namespace rexlib
{

template <typename Rule>
inline
const shape_rule_descriptor& shape_rule_descriptor_of<Rule>::get() noexcept
{
	static const shape_rule_descriptor descriptor(
		extent_pivot_descriptor_table<typename Rule::extent_list>::get(),
		core_dimensions_descriptor_table<
			typename Rule::output_core_list
		>::get(),
		core_dimensions_descriptor_table<
			typename Rule::input_core_list
		>::get(),
		Rule::placement_type::placement,
		Rule::batch
	);
	return descriptor;
}

} // namespace rexlib
