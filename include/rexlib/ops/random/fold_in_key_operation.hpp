// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/numerical_type_domain_tags.hpp>
#include <rexlib/core/operation/operation_declaration_macros.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/rules/shape_rules.hpp>
#include <rexlib/ops/rules/type_rules.hpp>

namespace rexlib
{
namespace ops
{

/**
 * @brief Derive keys from keys and values.
 *
 * The key and the data are uint64 arrays that broadcast against each other.
 * Each element of the result is the key derived from an element of the key
 * and the matching element of the data.
 *
 * The derived key is the first half of the Philox4x32-10 block keyed by the
 * key. Words 0 and 1 of the counter are the low and the high half of the
 * data, word 2 is 0 and word 3 is 1. The first half of the block joins its
 * words 0 and 1, with word 0 as the low half. Distinct values give
 * independent keys, and a derived key never shares a block with a draw of
 * its parent key.
 */
REXLIB_DECLARE_OPERATION(
	fold_in_key,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("key", "data"),
	binary_elementwise_shape_rule,
	binary_homogeneous_rule<single_type_domain<numerical_type::uint64>>
);

} // namespace ops
} // namespace rexlib
