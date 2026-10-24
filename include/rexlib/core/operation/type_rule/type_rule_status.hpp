// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

namespace rexlib
{

/**
 * @brief Outcome of resolving a typing rule against concrete operand types.
 */
enum class type_rule_status
{
	/**
	 * @brief The rule resolved successfully.
	 */
	ok,

	/**
	 * @brief A pivot took a value that is not a concrete numerical type.
	 */
	invalid_type,

	/**
	 * @brief A pivot took a value outside the domain the rule declares.
	 */
	domain_violation,

	/**
	 * @brief A transform is not defined on the value its pivot took.
	 */
	undefined_transform,

	/**
	 * @brief An operand type disagrees with the slot binding it.
	 */
	slot_mismatch,

	/**
	 * @brief A free pivot could not be determined from the operands.
	 */
	unresolved_pivot
};

/**
 * @brief Get a human readable representation of a rule status.
 *
 * @param status The status.
 * @return const char* The representation.
 */
REXLIB_API
const char* to_string(type_rule_status status) noexcept;

} // namespace rexlib
