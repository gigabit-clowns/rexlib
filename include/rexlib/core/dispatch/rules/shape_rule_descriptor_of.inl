// SPDX-License-Identifier: GPL-3.0-only

#include "shape_rule_descriptor_of.hpp"

#include "core_dimensions_descriptor_table.hpp"
#include "extent_descriptor_table.hpp"

namespace rexlib
{

template <typename Rule>
inline
const shape_rule_descriptor& shape_rule_descriptor_of<Rule>::get() noexcept
{
	static const shape_rule_descriptor descriptor(
		extent_descriptor_table<typename Rule::extent_list>::get(),
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
