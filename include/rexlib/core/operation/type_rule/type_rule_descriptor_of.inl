// SPDX-License-Identifier: GPL-3.0-only

#include "type_rule_descriptor_of.hpp"

#include <rexlib/core/operation/type_rule/type_pivot_descriptor_table.hpp>
#include <rexlib/core/operation/type_rule/type_slot_descriptor_table.hpp>

namespace rexlib
{

template <typename Rule>
inline
const type_rule_descriptor& type_rule_descriptor_of<Rule>::get() noexcept
{
	static const type_rule_descriptor descriptor(
		type_pivot_descriptor_table<typename Rule::pivot_list>::get(),
		type_slot_descriptor_table<typename Rule::output_slot_list>::get(),
		type_slot_descriptor_table<typename Rule::input_slot_list>::get()
	);
	return descriptor;
}

} // namespace rexlib
