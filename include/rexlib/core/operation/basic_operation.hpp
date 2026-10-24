// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/operand_names.hpp>
#include <rexlib/core/operation/operation.hpp>
#include <rexlib/core/operation/operation_descriptor.hpp>

#include <rexlib/core/platform/constexpr.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>

#include <cstddef>

namespace rexlib
{

/**
 * @brief CRTP base supplying the static description of an operation.
 *
 * Implements everything an operation can state once and for all: its name,
 * its arity and the names of its operands, all read from @p Traits. The
 * signature getter is left pure, so that an operation owning its signature
 * derives from this one and the compiler enforces that it provides it.
 *
 * The operand counts are exposed as compile time constants, which is what a
 * backend builder needs to size its operand arrays. They come from the
 * operation's typing rule and are checked against the operand name list and
 * the shape rule, so the arity cannot drift from any of them.
 *
 * @tparam Derived The concrete operation type. It keeps the base class type
 * distinct per operation, so that two operations sharing a rule do not
 * share a base.
 * @tparam Traits The static description. It must provide:
 *   - `static const char* get_component() noexcept`
 *   - `static const char* get_name() noexcept`
 *   - `static const operand_names<N>& get_output_names() noexcept`
 *   - `static const operand_names<M>& get_input_names() noexcept`
 *   - `using shape_rule_type = ...`
 *   - `using type_rule_type = ...`
 *
 * @see trivial_operation
 * @see parametric_operation
 * @see operand_shape_rule
 * @see operand_type_rule
 */
template <typename Derived, typename Traits>
class basic_operation
	: public operation
{
public:
	using traits_type = Traits;
	using shape_rule_type = typename Traits::shape_rule_type;
	using type_rule_type = typename Traits::type_rule_type;

	static REXLIB_CONST_CONSTEXPR std::size_t output_operand_count =
		type_rule_type::output_count;
	static REXLIB_CONST_CONSTEXPR std::size_t input_operand_count =
		type_rule_type::input_count;

	static_assert(
		output_operand_count == operand_names_count<
			decltype(Traits::get_output_names())
		>::value,
		"The operation declares a different number of output operand names "
		"than its typing rule has output slots."
	);

	static_assert(
		input_operand_count == operand_names_count<
			decltype(Traits::get_input_names())
		>::value,
		"The operation declares a different number of input operand names "
		"than its typing rule has input slots."
	);

	/**
	 * @brief Get the description shared by every instance.
	 *
	 * @return const operation_descriptor& The descriptor.
	 */
	static const operation_descriptor& get_static_descriptor() noexcept;

	/**
	 * @brief Get the description of this operation.
	 *
	 * @return const operation_descriptor& The descriptor.
	 */
	const operation_descriptor& get_descriptor() const noexcept override;

	/**
	 * @brief Get the identifier of @p Derived.
	 *
	 * Pinned to @p Derived rather than to the dynamic type, so that an
	 * operation specialising another still dispatches to the builders of
	 * the operation it specialises.
	 *
	 * @return operation_id The operation id.
	 */
	operation_id get_id() const noexcept override;
};

} // namespace rexlib

#include "basic_operation.inl"
