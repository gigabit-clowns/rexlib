// SPDX-License-Identifier: GPL-3.0-only

#include "shared_rule_contract.hpp"

#include <rexlib/core/operation/contract/rule_contract.hpp>

#include <rexlib/core/operation/no_parameters.hpp>

namespace rexlib
{

template <typename ShapeRule, typename TypeRule>
inline
const operand_contract&
shared_rule_contract<ShapeRule, TypeRule>::get() noexcept
{
	static const rule_contract<
		ShapeRule,
		TypeRule,
		no_parameters
	> instance{};
	return instance;
}

} // namespace rexlib
