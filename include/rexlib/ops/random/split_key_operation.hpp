// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/numerical_type_domain_tags.hpp>
#include <rexlib/core/operation/operation_declaration_macros.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/parameters/sequence_length.hpp>
#include <rexlib/ops/rules/shape_rules.hpp>
#include <rexlib/ops/rules/type_rules.hpp>

#include <cstddef>

namespace rexlib
{
namespace ops
{

REXLIB_DECLARE_PARAMETRIC_OPERATION_TRAITS(
	split_key,
	ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("key"),
	elementwise_sequence_shape_rule,
	sequence_length,
	unary_homogeneous_rule<single_type_domain<numerical_type::uint64>>
);

/**
 * @brief Derive a sequence of keys from every key.
 *
 * The key is a uint64 array. The result holds the shape of the key followed
 * by one axis of the length the operation names. Element `i` along that
 * axis is the key @ref fold_in_key_operation derives from the key and `i`.
 */
REXLIB_BEGIN_TEMPLATE_BASE
class REXLIB_API split_key_operation final
	: public parametric_operation<
		split_key_operation,
		split_key_operation_traits
	>
{
public:
	/**
	 * @brief Construct a split of every key into a number of keys.
	 *
	 * @param count The number of keys derived from every key.
	 */
	explicit split_key_operation(std::size_t count);

	/**
	 * @brief Get the number of keys derived from every key.
	 *
	 * @return std::size_t The count.
	 */
	std::size_t get_count() const noexcept;
};
REXLIB_END_TEMPLATE_BASE

} // namespace ops
} // namespace rexlib
