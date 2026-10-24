// SPDX-License-Identifier: GPL-3.0-only

#include "shared_rule_signature.hpp"

#include <rexlib/core/operation/rule_signature.hpp>

#include <rexlib/core/operation/no_parameters.hpp>

namespace rexlib
{

template <typename ShapeRule, typename TypeRule>
inline
const operation_signature&
shared_rule_signature<ShapeRule, TypeRule>::get() noexcept
{
	static const rule_signature<
		ShapeRule,
		TypeRule,
		no_parameters
	> instance{ no_parameters() };
	return instance;
}

} // namespace rexlib
