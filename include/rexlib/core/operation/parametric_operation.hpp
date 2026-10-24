// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/basic_operation.hpp>
#include <rexlib/core/operation/rule_signature.hpp>

#include <rexlib/core/platform/dynamic_shared_object.h>

#include <type_traits>
#include <utility>

namespace rexlib
{

/**
 * @brief CRTP base for operations whose signature depends on parameters.
 *
 * Owns a rule_signature holding the parameters of the operation
 * and forwards every constructor argument to them, so an operation whose
 * signature reads its parameters, such as a reduction over a set of axes,
 * needs no body of its own. The parameters are checked against the rules
 * on construction.
 *
 * Only parameters the signature reads are covered. An operation carrying a
 * value its signature does not read declares that value itself, as
 * fill_operation does.
 *
 * The signature is not copyable, so the operation is not either.
 *
 * @tparam Derived The concrete operation type.
 * @tparam Traits The static description. In addition to what
 * basic_operation requires, it must provide `using parameters_type = ...`
 * naming the type of the parameters, and its rules must read them.
 *
 * @see basic_operation
 * @see trivial_operation
 * @see rule_signature
 */
template <typename Derived, typename Traits>
class parametric_operation
	: public basic_operation<Derived, Traits>
{
public:
	using typename basic_operation<Derived, Traits>::shape_rule_type;
	using typename basic_operation<Derived, Traits>::type_rule_type;
	using parameters_type = typename Traits::parameters_type;
	using signature_type = rule_signature<
		shape_rule_type,
		type_rule_type,
		parameters_type
	>;

	static_assert(
		shape_rule_type::requires_parameters ||
		type_rule_type::requires_parameters,
		"The rules of a parametric operation read nothing from its "
		"parameters. Declare it as a trivial operation instead."
	);

	/**
	 * @brief Construct the operation by constructing its parameters.
	 *
	 * Constrained on the parameters being constructible from @p args, so
	 * that a wrong argument list is reported as no matching constructor
	 * rather than as an error inside the parameters, and so that this
	 * constructor does not take part in copy initialization.
	 *
	 * @tparam Args Constructor argument types of the parameters.
	 * @param args Forwarded to the parameters.
	 *
	 * @throws std::invalid_argument When the rules do not admit the
	 * parameters.
	 */
	template <
		typename... Args,
		typename = typename std::enable_if<
			std::is_constructible<parameters_type, Args...>::value
		>::type
	>
	explicit parametric_operation(Args&&... args);

	/**
	 * @brief Get the parameters of the operation.
	 *
	 * How a program builder reads them back. A builder is instantiated on
	 * the operation type, so it recovers the operation with operation_cast
	 * and never has to downcast the erased signature.
	 *
	 * @return const parameters_type& The parameters.
	 */
	const parameters_type& get_parameters() const noexcept;

	const operation_signature& get_signature() const noexcept override;

private:
	signature_type m_signature;
};

} // namespace rexlib

#include "parametric_operation.inl"
