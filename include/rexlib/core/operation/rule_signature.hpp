// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/shape_rule/shape_rule_interpreter.hpp>
#include <rexlib/core/operation/type_rule/type_rule_interpreter.hpp>

#include <rexlib/core/operation/operand_specification.hpp>
#include <rexlib/core/operation/operation_descriptor.hpp>
#include <rexlib/core/operation/operation_signature.hpp>
#include <rexlib/core/span.hpp>

#include <type_traits>

namespace rexlib
{

/**
 * @brief Operation signature made of a shape rule and a typing rule.
 *
 * Owns the parameters of an operation instance and reads from them, through
 * the accessors the rules name, whatever the rules depend on. The shape
 * rule decides the shapes of the operands and the typing rule their data
 * types, each without the other.
 *
 * @tparam ShapeRule The operand_shape_rule.
 * @tparam TypeRule The operand_type_rule.
 * @tparam Parameters The parameter type of the operation, no_parameters for
 * an operation without any.
 *
 * @see shared_rule_signature
 */
template <typename ShapeRule, typename TypeRule, typename Parameters>
class rule_signature final
	: public operation_signature
{
public:
	using parameters_type = Parameters;

	/**
	 * @brief Construct the signature by constructing its parameters.
	 *
	 * @tparam Args Constructor argument types of the parameters.
	 * @param args Forwarded to the parameters.
	 */
	template <
		typename... Args,
		typename = typename std::enable_if<
			std::is_constructible<Parameters, Args...>::value
		>::type
	>
	explicit rule_signature(Args&&... args);

	/**
	 * @brief Get the parameters of the operation instance.
	 *
	 * @return const Parameters& The parameters.
	 */
	const Parameters& get_parameters() const noexcept;

	/**
	 * @brief Check the parameters against the rules.
	 *
	 * Checks what does not depend on the operands, such as how many axes are
	 * named, so that an operation can reject its parameters when it is
	 * built rather than when it is first executed.
	 *
	 * @param descriptor Description of the operation, used to name it when
	 * reporting a rejection.
	 *
	 * @throws std::invalid_argument When a parameter is not admissible.
	 */
	void check_parameters(const operation_descriptor &descriptor) const;

	void deduce(
		const operation_descriptor &descriptor,
		span<operand_specification> canonical_outputs,
		span<const operand_specification> inputs
	) const override;

	void accept(
		const operation_descriptor &descriptor,
		span<const operand_specification> user_outputs,
		span<const operand_specification> canonical_outputs,
		span<const operand_specification> inputs
	) const override;

private:
	Parameters m_parameters;

	static shape_rule_interpreter get_shape_interpreter() noexcept;
	static type_rule_interpreter get_type_interpreter() noexcept;
};

} // namespace rexlib

#include "rule_signature.inl"
