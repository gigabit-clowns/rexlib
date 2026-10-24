// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/operation/type_rule/type_rule_status.hpp>

namespace rexlib
{

const char* to_string(type_rule_status status) noexcept
{
	switch (status)
	{
	case type_rule_status::ok:
		return "ok";
	case type_rule_status::invalid_type:
		return "invalid type";
	case type_rule_status::domain_violation:
		return "domain violation";
	case type_rule_status::undefined_transform:
		return "undefined transform";
	case type_rule_status::slot_mismatch:
		return "slot mismatch";
	case type_rule_status::unresolved_pivot:
		return "unresolved pivot";
	case type_rule_status::operand_count_mismatch:
		return "operand count mismatch";
	}

	return "unknown";
}

} // namespace rexlib
