// SPDX-License-Identifier: GPL-3.0-only

#include "rule_operation_data_type_policy.hpp"

namespace rex
{

template <typename Rule>
inline
const operation_data_type_policy&
rule_data_type_policy<Rule>::get() noexcept
{
	static const rule_operation_data_type_policy instance(
		pivot_descriptor_table<typename Rule::pivot_list>::get(),
		slot_descriptor_table<typename Rule::output_slot_list>::get(),
		slot_descriptor_table<typename Rule::input_slot_list>::get()
	);
	return instance;
}

} // namespace rex
